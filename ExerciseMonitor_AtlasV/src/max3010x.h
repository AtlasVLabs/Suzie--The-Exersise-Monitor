bool max30102_begin(void);
bool max30102_reset(void);

bool max30102_writeReg(unsigned char reg, unsigned char value);
unsigned char max30102_readReg(unsigned char reg);

bool max30102_configHR(void);
bool max30102_configSpO2(void);

void max30102_clearFIFO(void);
unsigned char max30102_availableSamples(void);

bool max30102_readSample(unsigned char *red, unsigned char *ir);
bool max30102_readFIFO(unsigned char *red, unsigned char *ir, int maxSamples);