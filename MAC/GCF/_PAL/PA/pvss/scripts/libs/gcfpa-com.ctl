void sendEventToPA(string msg, string destSysName)
{
	sendEvent(destSysName + "__gcf_DPA_server.", msg);
}

void sendEvent(string dest, string msg)
{
	blob event;
	blobZero(event, strlen(msg));
	blobSetValue(event, 0, msg, strlen(msg));
	dpSet(dest, event);
}

