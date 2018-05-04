#include "notificationlist.h"

static Notification list[NOTIFICATION_LIMIT] = { 0 };

static int firstEmptySlot()
{
	for (int i = 0; i < NOTIFICATION_LIMIT; i++) {
		if (list[i].created == 0) {
			return i;
		}
	}

	return -1;
}

Notification* ntls_create(const time_t created, const wchar_t* title, const wchar_t* body)
{
	// Find first empty slot
	int emptySlot = firstEmptySlot();
	if (emptySlot < 0) {
		ntfu_log("Out of usable notification slots!\n");
		return 0;
	}

	// Fill
	Notification* ntf = &list[emptySlot];
	ntf->index = emptySlot;
	ntf->created = created;
	ntf->title = _wcsdup(title);
	ntf->body = _wcsdup(body);

	return ntf;
}

void ntls_remove(Notification* notification)
{
	// Free members
	free(notification->title);
	free(notification->body);

	// Clear created
	notification->created = 0;
}
