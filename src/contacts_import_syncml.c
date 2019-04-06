
/*
 * Osmo - a handy personal organizer
 *
 * Copyright (C) 2007 Tomasz Maka <pasp@users.sourceforge.net>
 *           (C) 2008 Markus Dahms <mad@automagically.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "contacts_import_syncml.h"
#include "i18n.h"
#include "utils.h"
#include "utils_gui.h"
#include "contacts.h"
#include "contacts_import.h"
#include "contacts_import_vcard.h"
#include "options_prefs.h"

#ifdef CONTACTS_ENABLED

static gboolean _contacts_from_syncml (GUI *appGUI, OsmoSyncMLData *smldata);

/*------------------------------------------------------------------------------*/

void
syncml_handle_error (SmlError *smlerror, GUI *appGUI) {

gchar tmpbuf[BUFFER_SIZE];

    sprintf (tmpbuf, "SyncML: %s", smlErrorPrint(&smlerror));
    utl_gui_create_dialog (GTK_MESSAGE_ERROR, tmpbuf, GTK_WINDOW(appGUI->main_window));

	smlErrorDeref(&smlerror);
}

/*------------------------------------------------------------------------------*/

gboolean import_contacts_from_syncml (gint type, GUI *appGUI) 
{
	gboolean retval = FALSE;
	OsmoSyncMLData *smldata;

    /* try to load data */
    smldata = g_new0(OsmoSyncMLData, 1);
    smldata->gui = appGUI;
    switch(type) {
        case SYNCML_BLUETOOTH: /* Bluetooth */
            smldata->oc_type = SML_OBEX_TYPE_BLUETOOTH;
            smldata->url = g_strdup(config.import_bluetooth_address);
            smldata->port = config.import_bluetooth_channel;
            break;
        case SYNCML_USB:
            smldata->oc_type = SML_OBEX_TYPE_USB;
            smldata->url = g_strdup_printf("%u", config.import_usb_interface);
            break;
        default:
            smldata->oc_type = SML_OBEX_TYPE_UNKNOWN;
            break;
    }
    /* options */
    smldata->wbxml = config.import_binary_xml;
    retval = _contacts_from_syncml(appGUI, smldata);

	return retval;
}

/*------------------------------------------------------------------------------*/

static gboolean syncml_idle_cb(gpointer data)
{
	OsmoSyncMLData *smldata = (OsmoSyncMLData *)data;
	gboolean events_pending = FALSE;
	GSList *item;

	g_assert(smldata);

	for(item = smldata->sml_sessions; item != NULL; item = item->next) {
		g_assert(item->data);
		if(smlDsSessionCheck((SmlDsSession *)item->data))
			events_pending = TRUE;
			break;
	}

	if(!events_pending)
		if(smlManagerCheck(smldata->sml_manager))
				events_pending = TRUE;

	if(events_pending) {
		for(item = smldata->sml_sessions; item != NULL; item = item->next)
			smlDsSessionDispatch((SmlDsSession *)item->data);
		smlManagerDispatch(smldata->sml_manager);
	}

	return TRUE;
}

/*------------------------------------------------------------------------------*/

static SmlBool _sml_recv_change(SmlDsSession *dsession, SmlChangeType type,
	const char *uid, char *data, unsigned int size, const char *contenttype,
	void *userdata, SmlError **error)
{
	OsmoSyncMLData *smldata = (OsmoSyncMLData *)userdata;

	g_debug("SyncML: _sml_recv_change called:\n%s", data);
	g_assert(smldata);

	contacts_import_vcard((gchar *)data, smldata->gui);

	g_free(data);
	return TRUE;
}

/*------------------------------------------------------------------------------*/

static void _sml_recv_sync(SmlDsSession *dsession, unsigned int numchanges,
	void *userdata)
{
	g_debug("SyncML: _sml_recv_sync called: %d changes", numchanges);
}

/*------------------------------------------------------------------------------*/

static void _sml_recv_alert_reply(SmlSession *session, SmlStatus *status,
	void *userdata)
{
	g_debug("SyncML: _sml_recv_alert_reply called");
}

/*------------------------------------------------------------------------------*/

static SmlBool _sml_recv_alert(SmlDsSession *dsession, SmlAlertType type,
	const char *last, const char *next, void *userdata)
{
	SmlError *smlerror = NULL;
	/*OsmoSyncMLData *smldata = (OsmoSyncMLData *)userdata;*/

	g_debug("SyncML: _sml_recv_alert called: at %s, type %i, last %s, next %s",
		smlDsSessionGetLocation(dsession), type, last, next);

	if(!smlDsSessionSendAlert(dsession, SML_ALERT_SLOW_SYNC, last, next,
		_sml_recv_alert_reply, NULL, &smlerror)) 
        HANDLE_ERROR;
	return FALSE;
}

/*------------------------------------------------------------------------------*/

static void syncml_finish_session(OsmoSyncMLData *smldata)
{
	SmlError *smlerror;

	smlManagerQuit(smldata->sml_manager);

	/* disconnect */
	if(!smlTransportDisconnect(smldata->sml_client, NULL, &smlerror)) {
		g_warning("SyncML error: %s", smlErrorPrint(&smlerror));
		smlErrorDeref(&smlerror);
	} else {
		smlManagerStop(smldata->sml_manager);
		smlTransportFinalize(smldata->sml_client, &smlerror);
		smlTransportFree(smldata->sml_client);
	}

	g_idle_remove_by_data(smldata);
	if(smldata->url)
		g_free(smldata->url);
	g_free(smldata);
}

/*------------------------------------------------------------------------------*/

static void _sml_manager_event(SmlManager *manager, SmlManagerEventType type,
	 SmlSession *session, SmlError *error, void *userdata)
{
	OsmoSyncMLData *smldata = (OsmoSyncMLData *)userdata;

	g_debug("SyncML: _sml_manager_event called (%p)", userdata);
	g_assert(smldata);

	switch(type) {
		case SML_MANAGER_CONNECT_DONE:
			g_debug("SyncML Manager: connection succeeded");
			break;
		case SML_MANAGER_DISCONNECT_DONE:
			g_debug("SyncML Manager: connection ended");
			break;
		case SML_MANAGER_TRANSPORT_ERROR:
			/*g_warning("SyncML Manager: transport error: %s",*/
				/*smlErrorPrint(&error));*/
            syncml_handle_error (error, smldata->gui);
			/*syncml_finish_session(smldata);*/     // FIXME
			break;
		case SML_MANAGER_SESSION_NEW:
			g_debug("SyncML Manager: new session with ID %s, version %d",
				smlSessionGetSessionID(session),
				smlSessionGetVersion(session));
			break;
		case SML_MANAGER_SESSION_FINAL:
			g_debug("SyncML Manager: session %s reported final",
				smlSessionGetSessionID(session));
			smlSessionFlush(session, TRUE, NULL);
			break;
		case SML_MANAGER_SESSION_END:
			g_debug("SyncML Manager: session end");
			syncml_finish_session(smldata);
			break;
		case SML_MANAGER_SESSION_ERROR:
			g_warning("SyncML Manager: session error: %s",
				smlErrorPrint(&error));
			syncml_finish_session(smldata);
			break;
		case SML_MANAGER_SESSION_WARNING:
			g_warning("SyncML Manager: session warning: %s",
				smlErrorPrint(&error));
			break;
		case SML_MANAGER_SESSION_FLUSH:
			g_debug("SyncML Manager: session flush");
			break;
	}
}

/*------------------------------------------------------------------------------*/

static void _sml_ds_alert(SmlDsSession *dsession, void *userdata)
{
	OsmoSyncMLData *smldata = (OsmoSyncMLData *)userdata;

	g_debug("SyncML: _sml_ds_alert called");
	g_assert(smldata);

	smlDsSessionGetAlert(dsession, _sml_recv_alert, userdata);
	smlDsSessionGetSync(dsession, _sml_recv_sync, userdata);
	smlDsSessionGetChanges(dsession, _sml_recv_change, userdata);

	smlDsSessionRef(dsession);
	smldata->sml_sessions = g_slist_append(smldata->sml_sessions, dsession);
}

/*------------------------------------------------------------------------------*/

static gboolean _contacts_from_syncml(GUI *appGUI, OsmoSyncMLData *smldata)
{
	SmlTransportObexClientConfig smlocc;
	SmlAuthenticator *smlauth;
	SmlDevInf *smldevinf;
	SmlDevInfAgent *smlagent;
	SmlNotification *smlsan;
	SmlLocation *smlloc;
	SmlDsServer *smldsserver;
	SmlDevInfDataStore *smldids;
	SmlError *smlerror = NULL;

	/* OBEX configuration */
	smlocc.type = smldata->oc_type;
	smlocc.url = smldata->url;
	smlocc.port = smldata->port;

	g_debug("SyncML: trying to connect to %s channel %u", smlocc.url,
		smlocc.port);

	smldata->sml_client = smlTransportNew(SML_TRANSPORT_OBEX_CLIENT,
		&smlerror);
	if(!smldata->sml_client) 
        HANDLE_ERROR;

	smldata->sml_manager = smlManagerNew(smldata->sml_client, &smlerror);
	if(!smldata->sml_manager) 
        HANDLE_ERROR;
	smlManagerSetEventCallback(smldata->sml_manager, _sml_manager_event,
		smldata);

	smlauth = smlAuthNew(&smlerror);
	if(!smlauth) 
        HANDLE_ERROR;
	smlAuthSetEnable(smlauth, FALSE);
	if(!smlAuthRegister(smlauth, smldata->sml_manager, &smlerror))
		HANDLE_ERROR;

	smldevinf = smlDevInfNew("OsmoSyncML", SML_DEVINF_DEVTYPE_WORKSTATION,
		&smlerror);
	if(!smldevinf) 
        HANDLE_ERROR;

	smlagent = smlDevInfAgentNew(smldevinf, &smlerror);
	if(!smlagent) 
        HANDLE_ERROR;
	if(!smlDevInfAgentRegister(smlagent, smldata->sml_manager, &smlerror))
		HANDLE_ERROR;

	smlsan = smlNotificationNew(SML_SAN_VERSION_11, SML_SAN_UIMODE_UNSPECIFIED,
		SML_SAN_INITIATOR_USER, 1, "OsmoSyncML", "/",
		smldata->wbxml ? SML_MIMETYPE_WBXML : SML_MIMETYPE_XML,
		&smlerror);
	if(!smlsan)
        HANDLE_ERROR;

	smlloc = smlLocationNew("/tmp", NULL, &smlerror);
	if(!smlloc)
        HANDLE_ERROR;

	smldsserver = smlDsServerNew("text/x-vcard", smlloc, &smlerror);
	if(!smldsserver) 
        HANDLE_ERROR;
	if(!smlDsServerRegister(smldsserver, smldata->sml_manager, &smlerror))
		HANDLE_ERROR;
	smlDsServerSetConnectCallback(smldsserver, _sml_ds_alert, smldata);
	if(!smlDsServerAddSan(smldsserver, smlsan, &smlerror)) 
        HANDLE_ERROR;

	smldids = smlDevInfDataStoreNew(smlLocationGetURI(smlloc), &smlerror);
	if(!smldids) 
        HANDLE_ERROR;
	smlDevInfDataStoreSetRxPref(smldids, SML_ELEMENT_TEXT_VCARD, "2.1");
	smlDevInfDataStoreSetTxPref(smldids, SML_ELEMENT_TEXT_VCARD, "2.1");
	smlDevInfDataStoreSetSyncCap(smldids, SML_DEVINF_SYNCTYPE_TWO_WAY, TRUE);
	smlDevInfDataStoreSetSyncCap(smldids, SML_DEVINF_SYNCTYPE_SLOW_SYNC, TRUE);
	smlDevInfDataStoreSetSyncCap(smldids,
		SML_DEVINF_SYNCTYPE_SERVER_ALERTED_SYNC, TRUE);
	smlDevInfAddDataStore(smldevinf, smldids);

	if(!smlTransportInitialize(smldata->sml_client, &smlocc, &smlerror))
		HANDLE_ERROR;

	if(!smlManagerStart(smldata->sml_manager, &smlerror)) 
        HANDLE_ERROR;

	if(!smlTransportConnect(smldata->sml_client, &smlerror)) 
        HANDLE_ERROR;

	if(!smlNotificationSend(smlsan, smldata->sml_client, &smlerror))
		HANDLE_ERROR;

	smlNotificationFree(smlsan);

	/* register callback function */
	g_idle_add(syncml_idle_cb, smldata);

	return FALSE;
}

/*------------------------------------------------------------------------------*/

#endif  /* CONTACTS_ENABLED */

