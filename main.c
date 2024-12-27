#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef long long ll;
typedef struct { ll a, b; } ll2;
typedef struct { ll a, b, c; } ll3;
typedef struct { ll a, b, c, d; } ll4;

#define THROW(fmt, ...) (fprintf(stderr, "Error: " fmt " at line %d\n", ##__VA_ARGS__, __LINE__), exit(1))

/// BEGIN RAND
ll rand_seed = 0x123456789LL;

ll rand_next() {
    ll x = rand_seed;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return rand_seed = x;
}
/// END RAND

/// BEGIN LIST
typedef struct {
    int len; // <= cap
    int cap; // 2**n
    int l; // [0, cap)
    ll *data;
} list;

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

/// BEGIN TREAP
// 参考: https://shifth.hatenablog.com/entry/2015/05/10/101822
struct treap_node {
    void *value;
    struct treap_node *l, *r;
    ll pri;
    int rev;
    int len;
};
typedef struct treap_node treap_node;
typedef struct {
    treap_node *l;
    treap_node *r;
} treap_node_pair;

treap_node *treap_node_new(void *value) {
    treap_node *t = (treap_node *) malloc(sizeof(treap_node));
    t->value = value;
    t->l = t->r = NULL;
    t->pri = rand_next();
    t->rev = 0;
    t->len = 1;
    return t;
}
void treap_node_free(treap_node *t) {
    if (t->l) treap_node_free(t->l);
    if (t->r) treap_node_free(t->r);
    free(t);
}
void treap_node_push(treap_node *t) {
    if (!t) return;
    if (t->rev) {
        t->rev = 0;
        treap_node *x = t->l;
        t->l = t->r;
        t->r = x;
        if (t->l) t->l->rev = !t->l->rev;
        if (t->r) t->r->rev = !t->r->rev;
    }
}
void treap_node_update(treap_node *t) {
    treap_node_push(t);
    t->len = 1;
    if (t->l) t->len += t->l->len;
    if (t->r) t->len += t->r->len;
}
treap_node *treap_node_merge(treap_node *l, treap_node *r) {
    if (!l || !r) return l ? l : r ? r : NULL;
    if (l->pri > r->pri) {
        l->r = treap_node_merge(l->r, r);
        treap_node_update(l);
        return l;
    } else {
        r->l = treap_node_merge(l, r->l);
        treap_node_update(r);
        return r;
    }
}
const treap_node_pair treap_node_pair_null = {NULL, NULL};
treap_node_pair treap_node_split(treap_node *t, int at) {
    if (!t) return treap_node_pair_null;
    int l_len = t->l ? t->l->len : 0;
    if (at <= l_len) {
        treap_node_pair p = treap_node_split(t->l, at);
        t->l = p.r;
        p.r = t;
        return p;
    } else {
        treap_node_pair p = treap_node_split(t->r, at - l_len - 1);
        t->r = p.l;
        p.l = t;
        return p;
    }
}
treap_node *treap_node_at(treap_node *t, int at) {
    if (!t) return NULL;
    int l_len = t->l ? t->l->len : 0;
    if (at < l_len) {
        return treap_node_at(t->l, at);
    } else if (at == l_len) {
        return t;
    } else {
        return treap_node_at(t->r, at - l_len - 1);
    }
}
int treap_node_partition_point(treap_node *t, int (*f)(void *)) {
    if (!t) return 0;
    int l_len = t->l ? t->l->len : 0;
    if (f(t->value)) {
        return l_len + 1 + treap_node_partition_point(t->r, f);
    } else {
        return treap_node_partition_point(t->l, f);
    }
}

typedef struct {
    treap_node *root;
} treap;

treap *treap_new() {
    treap *t = (treap *) malloc(sizeof(treap));
    t->root = NULL;
    return t;
}
void treap_free(treap *t) {
    if (t->root) treap_node_free(t->root);
    free(t);
}
int treap_len(treap *t) {
    if (!t->root) return 0;
    return t->root->len;
}
void treap_insert(treap *t, int index, void *item) {
    if (!(index <= treap_len(t))) THROW("treap_insert(): index out of bounds");
    treap_node *node = treap_node_new(item);
    if (!t->root) {
        t->root = node;
    } else {
        treap_node_pair p = treap_node_split(t->root, index);
        t->root = treap_node_merge(p.l, treap_node_merge(node, p.r));
    }
}
int treap_partition_point(treap *t, int (*f)(void *)) {
    return treap_node_partition_point(t->root, f);
}
void *treap_temporary_item;
int (*treap_temporary_cmp)(void *, void *);
int treap_cmp_f(void *x) {
    return treap_temporary_cmp(x, treap_temporary_item) <= 0;
}
void treap_insert_sorted(treap *t, int (*cmp)(void *, void *), void *item) {
    treap_temporary_item = item;
    treap_temporary_cmp = cmp;
    int index = treap_partition_point(t, treap_cmp_f);
    treap_insert(t, index, item);
}
void *treap_remove(treap *t, int index) {
    if (!(index < treap_len(t))) THROW("treap_remove(): index out of bounds");
    treap_node_pair pl = treap_node_split(t->root, index);
    treap_node_pair pr = treap_node_split(pl.r, 1);
    t->root = treap_node_merge(pl.l, pr.r);
    void *item = pr.l->value;
    treap_node_free(pr.l);
    return item;
}
void **treap_at(treap *t, int index) {
    if (!(index < treap_len(t))) THROW("treap_at(): index out of bounds");
    return &treap_node_at(t->root, index)->value;
}
/// END TREAP

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