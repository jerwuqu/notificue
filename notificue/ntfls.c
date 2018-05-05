// Notification List

#include "ntfls.h"
#include "ntfshow.h"

static Notification root = { 0 };
static Notification* tail = &root;
static int notificationCount;

Notification* ntfls_create(const time_t created, const wchar_t* title, const wchar_t* body, SIZE boxSize)
{
	// Create
	Notification* ntf = (Notification*)malloc(sizeof(Notification));
	tail->_next = ntf;
	ntf->_prev = tail;
	tail = ntf;
	ntf->_next = NULL;
	ntf->index = notificationCount++;;
	ntf->created = created;
	ntf->title = _wcsdup(title);
	ntf->body = _wcsdup(body);
	ntf->boxSize = boxSize;
	ntf->boxYOffset = ntf->_prev->boxYOffset + ntf->_prev->boxSize.cy;

	return ntf;
}

void ntfls_remove(Notification* ntf)
{
	if (ntf == &root) return;

	// Remove from list
	notificationCount--;
	if (ntf == tail) {
		tail = ntf->_prev;
		tail->_next = NULL;
	} else {
		ntf->_prev->_next = ntf->_next;
		ntf->_next->_prev = ntf->_prev;

		// Move followers
		Notification* current = ntf;
		while (current != tail) {
			current = current->_next;
			current->index--;
			current->boxYOffset = current->_prev->boxYOffset + current->_prev->boxSize.cy;
			ntfshow_reposition(current);
		}
	}

	// Free
	free(ntf->title);
	free(ntf->body);
	free(ntf);
}
