/*
 * MRF24WG scan functions.
 *
 * Functions to initiate a scan and retrieve scan results.
 */
#include "wf_universal_driver.h"
#include "wf_global_includes.h"

/*
 * Commands the MRF24W to start a scan operation.  This will generate the
 * WF_EVENT_SCAN_RESULTS_READY event.
 *
 * Directs the MRF24W to initiate a scan operation utilizing the input
 * Connection Profile ID.  The Host Application will be notified that the scan
 * results are ready when it receives the WF_EVENT_SCAN_RESULTS_READY event.
 * The eventInfo field for this event will contain the number of scan results.
 * Once the scan results are ready they can be retrieved with
 * WF_ScanResultGet().
 *
 * Scan results are retained on the MRF24W until:
 *  1.  Calling WF_Scan() again (after scan results returned from previous
 *      call).
 *  2.  MRF24W reset.
 *
 * Parameters:
 *  mode - WF_SCAN_FILTERED or WF_SCAN_ALL
 *
 *    WF_SCAN_FILTERED:
 *          * If SSID defined only scan results with that SSID are retained
 *          * If SSID not defined all scanned SSID?s will be retained
 *          * Only scan results from Infrastructure or AdHoc networks are
 *             retained, depending on the configured network type
 *          * The only channels scanned are those set in WF_SetChannelList()
 *
 *    WF_SCAN_ALL:
 *          * Can be called after WF_Init() successfully completes (see WF_INIT_SUCCESSFUL eventData)
 *          * All scan results are retained (both Infrastructure and Ad Hoc networks).
 *          * All channels within the MRF24W's regional domain will be scanned.
 */
void WF_Scan(u_int8_t scanMode)
{
    int connectionState;
    u_int8_t hdr[4];

#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode = UdScan(scanMode);
    if (errorCode != UD_SUCCESS) {
        printf("--- %s: invalid scan mode=%u\n", __func__, scanMode);
        return;
    }
#endif

    // Can only scan when connected or idle
    connectionState = WF_ConnectionStateGet();
    if (connectionState == WF_CSTATE_CONNECTION_IN_PROGRESS ||
        connectionState == WF_CSTATE_RECONNECTION_IN_PROGRESS) {
        printf("--- %s: scan not allowed\n", __func__);
        return;
    }

    hdr[0] = WF_TYPE_MGMT_REQUEST;
    hdr[1] = WF_SUBTYPE_SCAN_START;
    hdr[2] = (scanMode == WF_SCAN_FILTERED) ? GetCpid() : 0xff;
    hdr[3] = 0;                                 /* not used */
    mrf_mgmt_send(hdr, sizeof(hdr), 0, 0, 1);
}

/*
 * Read scan results back from MRF24W.
 * The RSSI value in the scan result will range from 43 to 106
 *
 * After a scan has completed this function is used to read one or more of the
 * scan results from the MRF24W.  The scan results will be written
 * contiguously starting at p_scanResults (see tWFScanResult structure for
 * format of scan result).
 *
 * MACInit must be called first.  WF_EVENT_SCAN_RESULTS_READY event must have
 * already occurrerd.
 *
 * Parameters:
 *  list_index  - Index (0-based list) of the scan entry to retrieve.
 *  scan_result - Pointer to location to store the scan result structure
 */
void WF_ScanResultGet(u_int8_t list_index, t_scanResult *scan_result)
{
    u_int8_t hdr[4];

    hdr[0] = WF_TYPE_MGMT_REQUEST;
    hdr[1] = WF_SUBTYPE_SCAN_GET_RESULTS;
    hdr[2] = list_index;        /* scan result index to read from */
    hdr[3] = 1;                 /* number of results to read */

    /* Index 4 contains number of scan results returned,
     * index 5 is first byte of first scan result. */
    mrf_mgmt_send_receive(hdr, sizeof(hdr),
        (u_int8_t*) scan_result, sizeof(*scan_result), 5);

    /* fix up endianness for the two 16-bit values in the scan results */
    scan_result->beaconPeriod = htons(scan_result->beaconPeriod);
    scan_result->atimWindow   = htons(scan_result->atimWindow);
}
