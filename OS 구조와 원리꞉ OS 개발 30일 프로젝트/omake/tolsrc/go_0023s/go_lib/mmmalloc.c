/* copyright(C) 2002 H.Kawai (under KL-01).  */

#include <stdio.h>

void *GOL_memmanalloc(struct GOL_STR_MEMMAN *p, size_t size)
{
	struct GOL_STR_MEMMAN *b, *r;
	size += sizeof (struct GOL_STR_MEMMAN) + sizeof (struct GOL_STR_MEMMAN) - 1;
	size &= ~(sizeof (struct GOL_STR_MEMMAN) - 1);
	do {
		b = p;
		p = p->u.s.next;
		if (p == NULL)
			return NULL;
	} while (p->u.s.bytes < size);
	r = p;
	if (p->u.s.bytes == size) {
		b->u.s.next = p->u.s.next;
	} else {
		p += size / sizeof (struct GOL_STR_MEMMAN);
		p->u.s.next = r->u.s.next;
		p->u.s.bytes = r->u.s.bytes - size;
		b->u.s.next = p;
	}
	r->u.s.bytes = size;
	return r + 1;
}

void GOL_memmanfree(struct GOL_STR_MEMMAN *man, void *p0)
{
	struct GOL_STR_MEMMAN *b, *q = man;
	struct GOL_STR_MEMMAN *p = ((struct GOL_STR_MEMMAN *) p0) - 1;
	if (p0) {
		do {
			b = q;
			q = q->u.s.next;
			if (q == NULL)
				break;
		} while (q < p);
		/* b < p < q */
		b->u.s.next = p;
		p->u.s.next = q;
		if (q == p + p->u.s.bytes / sizeof (struct GOL_STR_MEMMAN)) {
			/* 상기 식이 성립한다면 물론 q != NULL이다 */
			p->u.s.bytes += q->u.s.bytes;
			p->u.s.next = q->u.s.next;
		}
		if (p == b + b->u.s.bytes / sizeof (struct GOL_STR_MEMMAN) && b ! = man) {
			b->u.s.bytes += p->u.s.bytes;
			b->u.s.next = p->u.s.next;
		}
	}
	return;
}

void *GOL_memmaninit(struct GOL_STR_MEMMAN *man, size_t size, void *p)
/* size는 충분히 클 것 */
/* p는 이미 충분히 align 되고 있을 것 */
{
	man->u.s.bytes = sizeof (struct GOL_STR_MEMMAN);
	man->u.s.next = p;
	((struct GOL_STR_MEMMAN *) p)->u.s.bytes = size;
	((struct GOL_STR_MEMMAN *) p)->u.s.next = NULL;
	return p;
}
