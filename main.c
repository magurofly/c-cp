#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef long long ll;
typedef struct { ll a, b; } ll2;
typedef struct { ll a, b, c; } ll3;
typedef struct { ll a, b, c, d; } ll4;

typedef struct {
    int len; // <= cap
    int cap; // 2**n
    int l; // [0, cap)
    ll *data;
} list;

#define THROW(fmt, ...) (fprintf(stderr, "Error: " fmt " at line %d\n", ##__VA_ARGS__, __LINE__), exit(1))

/// BEGIN LIST
list *list_new(int cap) {
    list *q = (list *) malloc(sizeof(list));
    q->cap = 1;
    while (q->cap < cap) q->cap *= 2;
    q->len = q->l = 0;
    q->data = (ll *) malloc(sizeof(ll) * q->cap);
    return q;
}

list *list_new_init(int len, ll item) {
    list *q = list_new(len);
    q->len = len;
    for (int i = 0; i < len; i++) q->data[i] = item;
    return q;
}

list *list_clone(const list *q) {
    list *r = list_new(q->cap);
    r->l = q->l;
    r->len = q->len;
    memcpy(r->data, q->data, sizeof(ll) * q->cap);
    return r;
}

void list_free(list *q) {
    free(q->data);
    free(q);
}

inline int list_empty(list *q) {
    return q->len == 0;
}

inline int list_len(list *q) {
    return q->len;
}

ll *list_at(list *q, int index) {
    if (index < 0) index += q->len;
    if (index < 0 || index >= q->len) THROW("list_at(): index out of bounds");
    return &q->data[index + q->l & q->cap - 1];
}

void list_resize(list *q, int new_len) {
    int new_cap = q->cap;
    while (new_cap < new_len) new_cap *= 2;
    ll *new_data = (ll *) malloc(sizeof(ll) * new_cap);
    if (q->l + q->len <= q->cap) {
        memcpy(&new_data[0], &q->data[q->l], sizeof(ll) * q->len);
    } else {
        int tail = q->cap - q->l;
        memcpy(&new_data[0], &q->data[q->l], sizeof(ll) * tail);
        memcpy(&new_data[tail], &q->data[0], sizeof(ll) * (q->len - tail));
    }
    q->l = 0;
    q->cap = new_cap;
    free(q->data);
    q->data = new_data;
}

void list_push(list *q, ll item) {
    if (q->len >= q->cap) list_resize(q, q->cap * 2);
    q->data[q->l + q->len++ & q->cap - 1] = item;
}

void list_unshift(list *q, ll item) {
    if (q->len >= q->cap) list_resize(q, q->cap * 2);
    q->len++;
    q->l = q->l - 1 & q->cap - 1;
    q->data[q->l] = item;
}

ll list_pop(list *q) {
    if (q->len <= 0) THROW("list_pop(): empty");
    return q->data[q->l + --q->len & q->cap - 1];
}

ll list_shift(list *q) {
    if (q->len <= 0) THROW("list_shift(): empty");
    ll item = q->data[q->l];
    q->len--;
    q->l = q->l + 1 & q->cap - 1;
    return item;
}

void list_print(list *q) {
    for (int i = 0; i < q->len; i++) printf("%lld%c", *list_at(q, i), (i != q->len - 1) ? ' ' : '\n');
}

ll list_sort_by_inner(ll *a, ll *b, int len, int (*f)(ll, ll)) {
    if (len <= 1) return 0;
    int mid = len / 2;
    ll inv = 0;
    inv += list_sort_by_inner(&a[0], &b[0], mid, f);
    inv += list_sort_by_inner(&a[mid], &b[mid], len - mid, f);
    int l = 0, r = mid, i = 0;
    while (l < mid && r < len) {
        if (f(a[l], a[r]) > 0) {
            inv += mid - l;
            b[i++] = a[r++];
        } else {
            b[i++] = a[l++];
        }
    }
    while (l < mid) b[i++] = a[l++];
    while (r < len) b[i++] = a[r++];
    memcpy(a, b, sizeof(ll) * len);
    return inv;
}
// ソートしてついでに転倒数を返す
ll list_sort_by(list *q, int (*f)(ll, ll)) {
    list_resize(q, q->len); // ひとつにまとめる
    ll *buf = (ll *) malloc(sizeof(ll) * q->cap);
    ll inv = list_sort_by_inner(q->data, buf, q->len, f);
    free(buf);
    return inv;
}
/// END LIST

int cmp(ll x, ll y) {
    return x - y;
}

int main(void) {
    list *xs = list_new_init(10, 0);
    printf("xs = %p\n", xs);
    for (int i = 0; i < 10; i++) *list_at(xs, i) =10 - i;
    
    list_print(xs);

    ll inv = list_sort_by(xs, cmp);
    
    list_print(xs);
    printf("inv = %lld\n", inv);

    return 0;
}