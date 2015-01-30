#ifndef INC_CONFIG_CONFIG_H
#define INC_CONFIG_CONFIG_H

struct config_output {
    void (*start) (void);
    void (*end) (void);
    void (*write_var) (const char *var, int set);
};

#endif
