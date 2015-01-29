#ifndef INCLUDE_IFACE_H
#define INCLUDE_IFACE_H

struct nclyr_pipes {
    int sig[2];
    int player[2];
    int lyr[2];
};

struct nclyr_iface {
    const char *name;
    const char *description;

    void (*main_loop) (struct nclyr_iface *, struct nclyr_pipes *);
};

extern struct nclyr_iface *nclyr_iface_list[];

struct nclyr_iface *nclyr_iface_find(const char *iface);

void nclyr_pipes_open(struct nclyr_pipes *);
void nclyr_pipes_close(struct nclyr_pipes *);

#endif
