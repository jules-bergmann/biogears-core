#include <biogears/engine/BioGearsPhysiologyEngine.h>
#include <biogears/cdm/utils/Logger.h>
#include <biogears/engine/Controller/BioGearsEngine.h>


// TRY THIS NEXT
// cPMML is C++ library for scoring machine learning models serialized with the Predictive Model Markup Language (PMML). It exposes a minimalist and user-friendly API and it targets high performance in model scoring, keeping a predictable and minimal memory footprint.
// Can this API be used in BioGears?
// https://amadeusitgroup.github.io/cPMML/installing.html#windows
// git clone https://github.com/AmadeusITGroup/cPMML.git && cd cPMML && install.bat

////////// NOTES ////////////////////////////////////////////////////////
// Found documentation referring to pmml importers using java, not useful for this
// From Stack Overflow, "You should be able to parse PMML with any XML library - there is nothing specific about PMML as an XML dialect." May be useful, but not sure if parsing will properly give us access to models in the file



using namespace biogears;

void StatePrediction(double heartRate_bpm, double sysArterialPressure_mmHg, double diaArterialPressure_mmHG, double cardiacOutput, double systemicVascularResistance, double bloodVolume_mL, double tissueIntegrity, double arterialBloodpH, double respirationRate_bpm, double tidalVolume_mL, double meanUrineOutput_mL_Per_hr, double urineProductionRate_ml_Per_hr, double skinTemp_degC) { 
    // Insert algorithm import here - Test simply first
    // Input is state from BG, output is representative state from prediction
        // Note: the way the pmml file works, the only input needed should be the featured parameters listed above. As a result, the input would be the parameters which makes it faster and easier to read in BG
    // Actually test input just being varibles needed...13 SME vars

    // PMML File here, need to get this to work for pmml library
    /*
    #include <iostream>
    #include "cPMML.h"
    std::cout << "cPMML version: " << cpmml::version << std::endl;
    */

    //return cluster to be used in next equation

}

void ProgressTimeDependentParameters(double time_s, int clusterNumber) {
  // Active event duration (always an hour), in seconds, parse active events and add 3600 s
  time_s += 3600;
  // The pmml file will assign a state prediction. Even if we only used the 13 variables in the function before, the pre-state is important here as it will have the action and event info needed here
  const int totalNumberClusters = 5; // Currenlty based on predicting state representation, then manually pushing one hour ahead
  // This could probably be streamlined by having another output in the .ipynb file that maps cluster centers and file names, but for first pass, it will be hard coded for results
  std::string centerKey[totalNumberClusters] = { "30Burn@345min_158Ringers_0Albumin.xml", 
      "23Burn@1185min_3Ringers_0Albumin.xml",
      "25Burn@390min_44Ringers_0Albumin.xml",
      "22Burn@285min_69Ringers_0Albumin.xml",
      "26Burn@900min_34Ringers_0Albumin.xml" };
  // Just putting in the one hour ahead states may be easier
  std::string hourAheadKey[totalNumberClusters] = { "30Burn@405min_158Ringers_0Albumin.xml",
                                                 "23Burn@1245min_3Ringers_0Albumin.xml",
                                                 "25Burn@450min_44Ringers_0Albumin.xml",
                                                 "22Burn@345min_69Ringers_0Albumin.xml",
                                                 "26Burn@960min_34Ringers_0Albumin.xml" };

  std::string centerAssignment = centerKey[clusterNumber];
  std::string futureAssignment = hourAheadKey[clusterNumber];

  // Adjust file from future state based on time input to equation (time_s)
  // Substance infusion, substance compound infusion, substance bolus (substance nasal and substance oral?) need to be modified
  // Rate in mL/hr, substract one hours worth from the bag volume. If less than zero, set to zero
}


int main(int argc, char* argv[]) {
  // Create the engine and load the patient
  Logger logger("HowToAsthma.log");
  std::unique_ptr<PhysiologyEngine> bg = CreateBioGearsEngine(&logger);

  bg->GetLogger()->Info("HowToAsthmaAttack");
  bg->SetAutoTrackFlag(true);
  bg->SetTrackStabilizationFlag(true);

  bg->GetSimulationTime(TimeUnit::min);

  //StatePrediction();
  ////At this point we should have a predicted cluster output from above. We need to translate that into a state file. Can simply hard code a map of clusters to files

  ////The selected file will then be passed in to here
  //ProgressTimeDependentParameters();

  if (!bg->InitializeEngine("patients/StandardMale.xml")) {
    bg->GetLogger()->Error("Could not load state, check the error");
    return 1;
  }
  return 0;
}