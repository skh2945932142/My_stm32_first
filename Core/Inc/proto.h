#ifndef __PROTO_H__
#define __PROTO_H__

#include <stdbool.h>
#include <stddef.h>

typedef struct {
  char type[8];
  int request_id;
  char module[16];
  char action[16];
  char payload[128];
} ProtoFrame;

bool Proto_ParseLine(char *line, ProtoFrame *out);
void Proto_Poll(void);
void Proto_HandleFrame(const ProtoFrame *frame);

void Proto_URLDecode(const char *input, char *output, size_t out_size);
int Proto_PayloadGetInt(const char *payload, const char *key, int default_value);
bool Proto_PayloadGetString(const char *payload, const char *key, char *out,
    size_t out_size);
bool Proto_PayloadGetDecodedString(const char *payload, const char *key,
    char *out, size_t out_size);

void Proto_SendAllCapabilities(void);
void Proto_SendAckOk(int request_id);
void Proto_SendAckErr(int request_id, const char *code, const char *message);
void Proto_SendCap(const char *module, const char *payload);
void Proto_SendStat(const char *module, const char *payload);
void Proto_SendEvt(const char *module, const char *action, const char *payload);
void Proto_SendLog(const char *level, const char *message);

#endif /* __PROTO_H__ */
