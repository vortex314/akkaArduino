#include <Log.h>
#include <Semaphore.h>
//________________________________________________________________ LINUX
//
#ifdef __linux__
#include <errno.h> /* errno, ECHILD            */
#include <fcntl.h>
#include <semaphore.h> /* sem_open(), sem_destroy(), sem_wait().. */
#include <semaphore.h>
#include <stdio.h>     /* printf()                 */
#include <stdlib.h>    /* exit(), malloc(), free() */
#include <sys/shm.h>   /* shmat(), IPC_RMID        */
#include <sys/types.h> /* key_t, sem_t, pid_t      */
#include <sys/types.h>

// use printf as log also uses semaphores

class LinuxSemaphore : public Semaphore {
		sem_t sema;
		uint32_t counter;

	public:
		LinuxSemaphore() {
			if (sem_init(&sema, 1, 1) < 0) {
				WARN("connect: Unable to create semaphore  errno : %d : %s ",errno, strerror(errno));
			}
			counter=0;
		}
		~LinuxSemaphore() {sem_destroy(&sema);}
		void wait() {
			if (::sem_wait(&sema) != 0) { /* P operation */
				printf("connect: Unable to wait semaphore  errno : %d : %s ",errno, strerror(errno));
			}
			counter++;
			if ( counter != 1 ) {
				int value;
				sem_getvalue(&sema, &value);
				printf("The value of the semaphors is %d\n", value);
				printf(" wait sema counter %d \n",counter);

			}
		}

		void release() {
			counter--;
			if ( counter != 0 ) {
				int value;
				sem_getvalue(&sema, &value);
				printf("The value of the semaphors is %d\n", value);
				printf(" wait sema counter %d \n",counter);

			}
			if (::sem_post(&sema) != 0) { /* V operation */
				printf("connect: Unable to post semaphore  errno : %d : %s ", errno, strerror(errno));
			}
		}
};

Semaphore& Semaphore::create() { return *new LinuxSemaphore(); }

#endif

#ifdef ARDUINO

class ArduinoSemaphore : public Semaphore {

	public:
		ArduinoSemaphore() {}
		~ArduinoSemaphore() {}
		void wait() {}

		void release() {}
};

Semaphore& Semaphore::create() {
	ArduinoSemaphore* ls = new ArduinoSemaphore();
	return *ls;
}

#endif
//____________________________________________________________ STM32
#ifdef STM32F1
class STM32Semaphore : public Semaphore {
		//    sem_t* sem;
		//    unsigned int value = 0;
		sem_t put_mutex;

	public:
		STM32Semaphore() {}
		void wait() { cm_disable_interrupts(); }

		void release() { cm_enable_interrupts(); }
};

Semaphore& Semaphore::create() { return *(new STM32Semaphore()); }
#endif

#if defined(ESP32_IDF) || defined(ESP_OPEN_RTOS)
#ifdef ESP_OPEN_RTOS
#include <FreeRTOS.h>
#include <semphr.h>
#endif
#ifdef ESP32_IDF
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#endif
class Esp32Semaphore : public Semaphore {
		SemaphoreHandle_t xSemaphore = NULL;
		uint32_t counter=0;
	public:
		Esp32Semaphore() {
			xSemaphore = xSemaphoreCreateBinary();
			xSemaphoreGive(xSemaphore);
		}
		~Esp32Semaphore() {}

		void wait() {
			if (xSemaphoreTake(xSemaphore, (TickType_t)1000) != pdTRUE) {
				printf(" xSemaphoreTake()  timed out ");
			}
			counter++;
			if ( counter != 1 ) {
				printf(" =======> wait sema counter %d \n",counter);
			}
		}

		void release() {
			if (xSemaphoreGive(xSemaphore) != pdTRUE) {
				printf("xSemaphoreGive() failed");
			}
			counter--;
			if ( counter != 0 ) {
				printf(" ======> give sema counter %d \n",counter);
			}

		}
//   static Semaphore& create() { return *(new Esp32Semaphore()); }
};

Semaphore& Semaphore::create() { return *new Esp32Semaphore(); }
#endif
