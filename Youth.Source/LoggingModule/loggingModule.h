#ifndef LOGGING_MODULE_H
#define LOGGING_MODULE_h

#ifdef __cplusplus
extern "C" {
#endif

// Initialize Logging Module
void initLoggingModule();

// Stop Logging Module
void stopLoggingModule();

// Start Record with file name
int startRecording(const char* filename);

// Stop Recording
void stopRecording();

// Start Logging file
int startPlayback(const char* filename);

// Stop Logging file
void stopRecording();

// Check Play Status
int isPlayingBack();

// Check Logging Module Execution Status 
int isLoggingModuleRunning();

#ifdef __cplusplus
}
#endif

#endif // LOGGING_MODULE_H
