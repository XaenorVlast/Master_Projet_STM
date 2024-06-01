

#ifndef INC_GESTION_H_
#define INC_GESTION_H_
#include <stdbool.h>
typedef struct {
    int maxAmplitudeZ;
    int minAmplitudeZ;
    int duration;
    int changeOfDirectionCount; // Ajout pour compter les changements de direction
} BenchRep;

typedef enum {
    INITIALISATION,
    ATTENTE_DEMARRAGE,
    ENREGISTREMENT_REFERENCE,
    VALIDATION_REFERENCE,
    ENREGISTREMENT_REPETITIONS,
	ATTENTE_SERIE
} ExerciceEtat;


// Prototypes de fonctions
void MX_MEMS_Init(void);
void MX_MEMS_Process();
void Init_Sensors(void);
void ReadSensorData(void);
void CalculateMovementAndSpeed(void);
void CalibrateSensors(void);
void CalibrateAccelerometer(void);
void CalibrateGyroscope(void);
bool recordBenchRep(BenchRep *rep);
bool compareBenchReps(BenchRep refRep, BenchRep newRep, int tolerance);
int _write(int file, char *ptr, int len);


extern ExerciceEtat etat;


#endif /* INC_GESTION_H_ */
