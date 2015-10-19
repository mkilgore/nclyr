#ifndef INC_CONFIG_CONFIG_H
#define INC_CONFIG_CONFIG_H

enum output_type {
    OUTPUT_YES,
    OUTPUT_NO,
    OUTPUT_QUOTE_STRING,
    OUTPUT_NOQUOTE_STRING,
};

struct config_output {
    void (*start) (void);
    void (*end) (void);
    void (*write_var) (enum output_type type, const char *prefix, const char *var, const char *dat);
};

#endif
