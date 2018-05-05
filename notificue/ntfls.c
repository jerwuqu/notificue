// Notification List

#include "ntfls.h"

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

Notification* ntfls_create(const time_t created, const wchar_t* title, const wchar_t* body, SIZE boxSize)
{
	// Find first empty slot
	int emptySlot = firstEmptySlot();
	if (emptySlot < 0) {
		log_text("Out of usable notification slots!\n");
		return 0;
	}

	// Fill
	Notification* ntf = &list[emptySlot];
	ntf->index = emptySlot;
	ntf->created = created;
	ntf->title = _wcsdup(title);
	ntf->body = _wcsdup(body);
	ntf->boxSize = boxSize;
	if (emptySlot > 0) {
		ntf->boxYOffset = list[emptySlot - 1].boxYOffset + list[emptySlot - 1].boxSize.cy;
	}

	return ntf;
}

void ntfls_remove(Notification* notification)
{
	// Free members
	free(notification->title);
	free(notification->body);

	// Clear created
	notification->created = 0;
}
