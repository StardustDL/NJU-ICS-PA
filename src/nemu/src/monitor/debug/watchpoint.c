#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "nemu.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool()
{
	int i;
	for (i = 0; i < NR_WP; i++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

static void clearWP(WP * wp)
{
	wp->next = NULL;
}

static WP *new_wp()
{
	if (free_ == NULL) {
		Error("No more watch points");
		assert(0);
	}
	WP *ret = free_->next;
	free_ = free_->next;
	clearWP(ret);
	return ret;
}

static void free_wp(WP * wp)
{
	wp->next = free_;
	free_ = wp;
}

WP *getHeadWP()
{
	return head;
}

WP *createWP()
{
	WP *cur = new_wp();
	if (head == NULL)
		head = cur;
	else {
		cur->next = head;
		head = cur;
	}
	return cur;
}

void removeWP(int no)
{
	for (WP * cur = head, *last = NULL; cur != NULL;
	     last = cur, cur = cur->next) {
		if (cur->NO != no)
			continue;
		if (cur == head) {
			head = cur->next;
			free_wp(cur);
			return;
		}
		else {
			last->next = cur->next;
			free_wp(cur);
			return;
		}
	}
}
