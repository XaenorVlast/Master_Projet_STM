#include "stm32wbxx_hal.h"
#include "iks01a3_mems_control.h"
#include "iks01a3_motion_sensors.h"
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Gestion.h"
#include <limits.h>

#include"app_ble.h"

// Définitions pour la configuration des capteurs
#define ALGO_FREQ 50U
#define ACC_ODR ((float)ALGO_FREQ)
#define ACC_FS 4
#define CALIBRATION_SAMPLES 100
#define MOVEMENT_THRESHOLD 100 // Seuil de détection de mouvement pour l'accéléromètre
#define REPETITION_TIME_OUT 2000 // Délai en millisecondes pour déterminer la fin d'une répétition
#define TOLERANCE 600 // Tolérance pour la comparaison des répétitions

// Variables globales
static MOTION_SENSOR_Axes_t AccValue;
static MOTION_SENSOR_Axes_t GyrValue;
static MOTION_SENSOR_Axes_t AccOffset;
static MOTION_SENSOR_Axes_t GyrOffset;
BenchRep referenceMovement;
BenchRep currentMovement;
ExerciceEtat etat = INITIALISATION;
bool isExerciseStarted = false;
bool exerciseEnded = false;
bool fin_seance = false;
int validMovements = 0;
int invalidMovements = 0;
uint32_t zeroForceValue;
uint32_t attention = 0;
extern ADC_HandleTypeDef hadc1;

int _write(int file, char *ptr, int len) {
	int DataIdx;
	for (DataIdx = 0; DataIdx < len; DataIdx++) {
		ITM_SendChar(*ptr++);
	}
	return len;
}

void MX_MEMS_Init(void) {
	Init_Sensors();
	CalibrateSensors();
	// Initialisation des structures BenchRep
	referenceMovement.maxAmplitudeZ = 0;
	referenceMovement.minAmplitudeZ = 0;
	referenceMovement.duration = 0;

	currentMovement.maxAmplitudeZ = 0;
	currentMovement.minAmplitudeZ = 0;
	currentMovement.duration = 0;

}

void MX_MEMS_Process() {
	switch (etat) {
	case INITIALISATION:
		// Initialisation des variables
		validMovements = 0;
		invalidMovements = 0;
		MX_MEMS_Init();
		isExerciseStarted = false;
		exerciseEnded = false;
		etat = ATTENTE_DEMARRAGE;
		break;

	case ATTENTE_DEMARRAGE:
		// Attente du signal pour démarrer un nouvel exercice
		if (!isExerciseStarted) {
			printf(
					"Nouvel exercice détecté, préparation à l'enregistrement...\n");
			etat = ENREGISTREMENT_REFERENCE;
		}
		break;

	case ENREGISTREMENT_REFERENCE:
		// Enregistrement du mouvement de référence
		printf("Enregistrement du mouvement...\n");
		if (!recordBenchRep(&referenceMovement)) {
			printf(
					"Aucun mouvement de référence détecté, fin de la tentative.\n");
			MVT_REF_vide();
			etat = INITIALISATION;
		} else {
			printf(
					"Mouvement de référence enregistré. En attente de validation...\n");
			MVT_REF_fin();
			etat = VALIDATION_REFERENCE;
		}
		break;

	case VALIDATION_REFERENCE:
		// Validation du mouvement de référence
		printf("Enregistrement du mouvement pour validation...\n");
		if (!recordBenchRep(&currentMovement)) {
			printf(
					"Aucun mouvement détecté pour validation, veuillez réessayer.\n");
			MVT_REF_validation_vide();
			etat = ENREGISTREMENT_REFERENCE;
		} else if (compareBenchReps(referenceMovement, currentMovement,
		TOLERANCE)) {
			printf("Validation réussie. Commencement des répétitions.\n");

			MVT_REF_validation_succes();
			etat = ENREGISTREMENT_REPETITIONS;
		} else {
			printf(
					"Validation échouée, veuillez réenregistrer le mouvement de référence.\n");
			MVT_REF_validation_fail();
			etat = ENREGISTREMENT_REFERENCE;
		}
		break;

	case ENREGISTREMENT_REPETITIONS:
		// Enregistrement des répétitions jusqu'à la fin de la série
		if (!exerciseEnded) {
			printf("Enregistrement d'une nouvelle répétition...\n");
			if (!recordBenchRep(&currentMovement)) {
				printf(
						"Fin de la série détectée après une période d'inactivité.\n");
				MVT_vide();
				exerciseEnded = true;
				etat = ATTENTE_SERIE;
			} else if (compareBenchReps(referenceMovement, currentMovement,
			TOLERANCE) && attention == 1) {
				MVT_BRAS_GAUCHE_valide();
				validMovements++;
				printf("Répétition valide.\n");

			} else if (compareBenchReps(referenceMovement, currentMovement,
			TOLERANCE) && attention == 2) {
				MVT_BRAS_DROIT_valide();
				validMovements++;
				printf("Répétition valide.\n");

			} else if (compareBenchReps(referenceMovement, currentMovement,
			TOLERANCE) && attention == 0) {
				MVT_BRAS_CORRECT_valide();
				validMovements++;
				printf("Répétition valide.\n");

			}


			else if (!compareBenchReps(referenceMovement, currentMovement,
			TOLERANCE) && attention == 1) {
				MVT_BRAS_GAUCHE_non_valide();
				invalidMovements++;
				printf("Répétition non valide.\n");



			} else if (!compareBenchReps(referenceMovement, currentMovement,
			TOLERANCE) && attention == 2) {
				MVT_BRAS_DROIT_non_valide();
				invalidMovements++;
				printf("Répétition non valide.\n");


			}

			else if (!compareBenchReps(referenceMovement, currentMovement,
			TOLERANCE) && attention == 0) {
				MVT_BRAS_CORRECT_non_valide();
				invalidMovements++;
				printf("Répétition non valide.\n");


			}

		}
		break;

	case ATTENTE_SERIE:
		// Attente d'une nouvelle série
		if (exerciseEnded) {
			printf("Prêt pour une nouvelle série.\n");
			exerciseEnded = false;
			etat = ENREGISTREMENT_REPETITIONS;
		}
		break;
	}
}

void Init_Sensors(void) {
	BSP_SENSOR_ACC_Init();
	BSP_SENSOR_GYR_Init();

	BSP_SENSOR_ACC_SetOutputDataRate(ACC_ODR);
	BSP_SENSOR_ACC_SetFullScale(ACC_FS);
}

void ReadSensorData(void) {
	BSP_SENSOR_ACC_GetAxes(&AccValue);
	BSP_SENSOR_GYR_GetAxes(&GyrValue);

// Appliquer le décalage après la calibration
	AccValue.x -= AccOffset.x;
	AccValue.y -= AccOffset.y;
	AccValue.z -= AccOffset.z;

	GyrValue.x -= GyrOffset.x;
	GyrValue.y -= GyrOffset.y;
	GyrValue.z -= GyrOffset.z;
}

void CalculateMovementAndSpeed(void) {
	printf("Accéléromètre [X: %ld, Y: %ld, Z: %ld]\n", (long) AccValue.x,
			(long) AccValue.y, (long) AccValue.z);
	printf("Gyroscope [X: %ld, Y: %ld, Z: %ld]\n", (long) GyrValue.x,
			(long) GyrValue.y, (long) GyrValue.z);
	HAL_Delay(2000);
}

void CalibrateSensors(void) {
	CalibrateAccelerometer();
	CalibrateGyroscope();
}

void CalibrateAccelerometer(void) {
	int sumX = 0, sumY = 0, sumZ = 0;
	MOTION_SENSOR_Axes_t tempVal;

	for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
		BSP_SENSOR_ACC_GetAxes(&tempVal);
		sumX += tempVal.x;
		sumY += tempVal.y;
		sumZ += tempVal.z;
		HAL_Delay(10);
	}

	AccOffset.x = sumX / CALIBRATION_SAMPLES;
	AccOffset.y = sumY / CALIBRATION_SAMPLES;
	AccOffset.z = sumZ / CALIBRATION_SAMPLES;
}

void CalibrateGyroscope(void) {
	int sumX = 0, sumY = 0, sumZ = 0;
	MOTION_SENSOR_Axes_t tempVal;

	for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
		BSP_SENSOR_GYR_GetAxes(&tempVal);
		sumX += tempVal.x;
		sumY += tempVal.y;
		sumZ += tempVal.z;
		HAL_Delay(10);
	}

	GyrOffset.x = sumX / CALIBRATION_SAMPLES;
	GyrOffset.y = sumY / CALIBRATION_SAMPLES;
	GyrOffset.z = sumZ / CALIBRATION_SAMPLES;
}

bool recordBenchRep(BenchRep *rep) {
	int maxValueZ = INT_MIN;
	int minValueZ = INT_MAX;
	uint32_t startTime = HAL_GetTick();
	uint32_t lastMovementTime = startTime;
	uint32_t currentTime;
	bool movementDetected = false;
	bool isAscending = false; // Initialisation différée
	int changeOfDirectionCount = 0;
	bool directionDetermined = false;
	uint32_t inactivityStartTime = HAL_GetTick(); // Pour détecter l'inactivité initiale
	zeroForceValue = Get_Calibration_Value();
	attention = 0;

	while (true) {
		ReadSensorData();

		// Détecter l'inactivité initiale
		if (!movementDetected
				&& (HAL_GetTick() - inactivityStartTime > 15000)) {
			printf(
					"Aucun mouvement détecté pendant plus de 15 secondes, fin de la tentative.\n");
			return false;
		}

		// Vérifier si le mouvement dépasse le seuil
		if (abs(AccValue.z) > MOVEMENT_THRESHOLD) {
			if (!movementDetected) {
				printf(
						"Mouvement détecté pour la première fois, début du suivi.\n");
				movementDetected = true;
				startTime = HAL_GetTick();
				lastMovementTime = startTime;
				inactivityStartTime = 0; // Réinitialiser puisqu'un mouvement a été détecté
			} else {
				lastMovementTime = HAL_GetTick();
			}

			// Mise à jour des valeurs maximales et minimales
			maxValueZ = fmax(maxValueZ, AccValue.z);
			minValueZ = fmin(minValueZ, AccValue.z);
			uint32_t adcValue = Read_ADC_Value();
			int32_t adjustedValue = (int32_t) adcValue
					- (int32_t) zeroForceValue;

			if (adjustedValue >= 3) {
				printf("Attention bras droit!\n");
				attention = 2;
			} else if (adjustedValue <= -3) {
				printf("Attention bras gauche!\n");
				attention = 1;
			}

			// Déterminer la direction du mouvement
			if (!directionDetermined) {
				isAscending = AccValue.z > 0;
				directionDetermined = true;

			} else {
				bool currentAscending = AccValue.z > 0;
				if (currentAscending != isAscending) {
					changeOfDirectionCount++;
					isAscending = currentAscending;

				}
			}
		}

		// Vérifier le temps d'inactivité
		currentTime = HAL_GetTick();
		if (movementDetected
				&& (currentTime - lastMovementTime > REPETITION_TIME_OUT)) {
			printf(
					"Période d'inactivité détectée, fin de l'enregistrement du mouvement.\n");
			break;
		}
	}

// Enregistrer les données du mouvement
	rep->maxAmplitudeZ = maxValueZ;
	rep->minAmplitudeZ = minValueZ;
	rep->duration = currentTime - startTime;
	rep->changeOfDirectionCount = changeOfDirectionCount;

	return true; // Mouvement détecté et enregistré
}

bool compareBenchReps(BenchRep refRep, BenchRep newRep, int tolerance) {
	if (abs(refRep.maxAmplitudeZ - newRep.maxAmplitudeZ) <= tolerance
			&& abs(refRep.minAmplitudeZ - newRep.minAmplitudeZ) <= tolerance
			&& abs(refRep.duration - newRep.duration) <= tolerance
			&& refRep.changeOfDirectionCount == newRep.changeOfDirectionCount) {
		return true;
	}
	return false;
}

uint32_t Read_ADC_Value(void) {
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	uint32_t adcValue = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	return adcValue;
}
uint32_t Get_Calibration_Value(void) {
	uint32_t sum = 0;
	const int numSamples = 10;
	for (int i = 0; i < numSamples; i++) {
		sum += Read_ADC_Value();
		HAL_Delay(10);  // Delay to allow ADC to stabilize
	}
	return sum / numSamples;
}
