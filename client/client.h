typedef  void(*maschine_callback)(uint32_t *data, size_t length);

void doHandshake();
void registerCallback(maschine_callback callback);