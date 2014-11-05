#ifdef GPU_KENEL
const int kWidth = 1366;
const int kHeight = 768;

size_t global_work_size = kWidth * kHeight;
size_t local_work_size = 1;
#endif
