#include "loggingModule.h"

extern pthread_mutex_t mutex;
extern pthread_cond_t cond;
extern int iEndOfSavingData;
 
extern int iNumOfPoint;
extern char* pcSensorData;
extern int astra_width;
extern int astra_height;

void receive_data(){
    // Receive buffer from sensor module
    
    int width, height;
    static int iTestCnt = 0;
    int16_t* depthData;
    uint8_t* colorData;

    pthread_mutex_lock(&mutex);
    while(!iEndOfSavingData){
        pthread_cond_wait(&cond, &mutex);
    }

    memcpy(&width, pcSensorData, sizeof(int));
    memcpy(&height, pcSensorData + sizeof(int), sizeof(int));

    // depthData는 크기가 int16_t이므로 그 크기에 맞게 복사
    depthData = (int16_t*)malloc(sizeof(int16_t) * width * height);
    if (depthData == NULL) {
        printf("malloc error for depthData\n");
        pthread_mutex_unlock(&mutex);
        return;
    }

    memcpy(depthData, pcSensorData + sizeof(int) * 2, sizeof(int16_t) * width * height);

    // colorData는 크기가 uint8_t이므로 그 크기에 맞게 복사
    colorData = (uint8_t*)malloc(sizeof(uint8_t) * width * height * 3);
    if (colorData == NULL) {
        printf("malloc error for colorData\n");
        free(depthData);  // 이미 할당된 depthData 메모리 해제
        pthread_mutex_unlock(&mutex);
        return;
    }
    
    memcpy(colorData, pcSensorData + sizeof(int) * 2 + sizeof(int16_t) * width * height, sizeof(uint8_t) * width * height * 3);

    pthread_mutex_unlock(&mutex);
    
    if(depthData && colorData){
        for(int y = 0; y < height; ++y){
            for(int x = 0; x < width; ++x){
                int index = y * width + x;
                int depthValue = depthData[index];

                if(depthValue > 0){
                    // Calculate 3D Coordinate (Using Simple Camera Model)
                    float z_pos = depthValue / 1000.0f; // from mm to m
                    float x_pos = (x - width / 2) * z_pos
                    / 570.3f; // 570.3f is focal distance of Astra camera
                    float y_pos = (y - height / 2) * z_pos
                    / 570.3f;

                    // Set color using color data (RGB order)
                    int colorIndex = index * 3; // RGB consist of 3 values.
                    float r = colorData[colorIndex] / 255.0f;
                    float g = colorData[colorIndex + 1] / 255.0f;
                    float b = colorData[colorIndex + 2] / 255.0f;

                    glColor3f(r, g, b);
                    glVertex3f(x_pos, -y_pos, -z_pos);
                    // printf("%lf %lf %lf %lf %lf %lf\n", x_pos, -y_pos, -z, r, g, b);
                }
            }
        }
    }
}

void* loggingModule(void* id){
    /* To do */

    while(1){
        // printf("Logging System Will Be Implemented Here.\n");
        receive_data();
    }
}