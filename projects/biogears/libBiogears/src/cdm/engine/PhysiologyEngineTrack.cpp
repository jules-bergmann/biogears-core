/**************************************************************************************
Copyright 2015 Applied Research Associates, Inc.
Licensed under the Apache License, Version 2.0 (the "License"); you may not use
this file except in compliance with the License. You may obtain a copy of the License
at:
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software distributed under
the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.
**************************************************************************************/
#include <biogears/cdm/engine/PhysiologyEngineTrack.h>
#include <biogears/engine/Systems/Nervous.h>
#include <biogears/engine/Systems/Cardiovascular.h>
#include <biogears/engine/Systems/Energy.h>
#include <biogears/engine/Systems/BloodChemistry.h>

#include <biogears/cdm/compartment/SECompartmentManager.h>
#include <biogears/cdm/compartment/fluid/SEGasCompartment.h>
#include <biogears/cdm/compartment/fluid/SEGasCompartmentLink.h>
#include <biogears/cdm/compartment/fluid/SELiquidCompartment.h>
#include <biogears/cdm/compartment/fluid/SELiquidCompartmentLink.h>
#include <biogears/cdm/engine/PhysiologyEngine.h>
#include <biogears/cdm/patient/SEPatient.h>
#include <biogears/cdm/properties/SEScalarAmountPerVolume.h>
#include <biogears/cdm/properties/SEScalarElectricPotential.h>
#include <biogears/cdm/properties/SEScalarFraction.h>
#include <biogears/cdm/properties/SEScalarFrequency.h>
#include <biogears/cdm/properties/SEScalarMass.h>
#include <biogears/cdm/properties/SEScalarMassPerVolume.h>
#include <biogears/cdm/properties/SEScalarPressure.h>
#include <biogears/cdm/properties/SEScalarTime.h>
#include <biogears/cdm/properties/SEScalarVolume.h>
#include <biogears/cdm/properties/SEScalarVolumePerTime.h>
#include <biogears/cdm/substance/SESubstance.h>
#include <biogears/cdm/substance/SESubstanceManager.h>
#include <biogears/cdm/substance/SESubstanceTissuePharmacokinetics.h>
#include <biogears/cdm/system/environment/SEEnvironment.h>
#include <biogears/cdm/system/equipment/Anesthesia/SEAnesthesiaMachine.h>
#include <biogears/cdm/system/equipment/ElectroCardioGram/SEElectroCardioGram.h>
#include <biogears/cdm/system/equipment/Inhaler/SEInhaler.h>
#include <biogears/cdm/system/physiology/SEBloodChemistrySystem.h>
#include <biogears/cdm/system/physiology/SECardiovascularSystem.h>
#include <biogears/cdm/system/physiology/SEDrugSystem.h>
#include <biogears/cdm/system/physiology/SEEndocrineSystem.h>
#include <biogears/cdm/system/physiology/SEEnergySystem.h>
#include <biogears/cdm/system/physiology/SEGastrointestinalSystem.h>
#include <biogears/cdm/system/physiology/SEHepaticSystem.h>
#include <biogears/cdm/system/physiology/SENervousSystem.h>
#include <biogears/cdm/system/physiology/SERenalSystem.h>
#include <biogears/cdm/system/physiology/SERespiratorySystem.h>
#include <biogears/cdm/system/physiology/SETissueSystem.h>
#include <biogears/cdm/utils/DataTrack.h>
#include <biogears/engine/BioGearsPhysiologyEngine.h>
#include <biogears/xver.h>

namespace std {
template class vector<biogears::SESystem*>;
template class map<const biogears::SEDataRequest*, biogears::SEDataRequestScalar*>;
}

namespace biogears {

std::string Space2Underscore(const std::string& str)
{
  std::string s = str;
  std::transform(s.begin(), s.end(), s.begin(), [](char ch) {
    return ch == ' ' ? '_' : ch;
  });
  return s;
}

PhysiologyEngineTrack::PhysiologyEngineTrack(PhysiologyEngine& engine)
  : Loggable(engine.GetLogger())
  , m_DataRequestMgr(engine.GetLogger())
  , m_Patient(&(SEPatient&)engine.GetPatient())
  , m_SubMgr(&(SESubstanceManager&)engine.GetSubstanceManager())
  , m_CmptMgr(&(SECompartmentManager&)engine.GetCompartments())
  , m_DataTrack(engine.GetLogger())
{

  // TODO We are not handling nullptr well here...

  SEBloodChemistrySystem* bchem = (SEBloodChemistrySystem*)engine.GetBloodChemistrySystem();
  if (bchem != nullptr)
    m_PhysiologySystems.push_back(bchem); // 0
  SECardiovascularSystem* cv = (SECardiovascularSystem*)engine.GetCardiovascularSystem();
  if (cv != nullptr)
    m_PhysiologySystems.push_back(cv); // 1
  SEEndocrineSystem* endo = (SEEndocrineSystem*)engine.GetEndocrineSystem();
  if (endo != nullptr)
    m_PhysiologySystems.push_back(endo);
  SEEnergySystem* energy = (SEEnergySystem*)engine.GetEnergySystem();
  if (energy != nullptr)
    m_PhysiologySystems.push_back(energy); // 3
  SERenalSystem* renal = (SERenalSystem*)engine.GetRenalSystem();
  if (renal != nullptr)
    m_PhysiologySystems.push_back(renal);
  SEGastrointestinalSystem* gi = (SEGastrointestinalSystem*)engine.GetGastrointestinalSystem();
  if (gi != nullptr)
    m_PhysiologySystems.push_back(gi);
  SERespiratorySystem* resp = (SERespiratorySystem*)engine.GetRespiratorySystem();
  if (resp != nullptr)
    m_PhysiologySystems.push_back(resp);
  SEDrugSystem* drug = (SEDrugSystem*)engine.GetDrugSystem();
  if (drug != nullptr)
    m_PhysiologySystems.push_back(drug);
  SETissueSystem* tissue = (SETissueSystem*)engine.GetTissueSystem();
  if (tissue != nullptr)
    m_PhysiologySystems.push_back(tissue);
  SENervousSystem* nervous = (SENervousSystem*)engine.GetNervousSystem();
  if (nervous != nullptr)
    m_PhysiologySystems.push_back(nervous);
  SEHepaticSystem* hepatic = (SEHepaticSystem*)engine.GetHepaticSystem();
  if (hepatic != nullptr)
    m_PhysiologySystems.push_back(hepatic);

  m_Environment = (SEEnvironment*)engine.GetEnvironment();

  SEAnesthesiaMachine* am = (SEAnesthesiaMachine*)engine.GetAnesthesiaMachine();
  if (am != nullptr)
    m_EquipmentSystems.push_back(am);
  SEElectroCardioGram* ecg = (SEElectroCardioGram*)engine.GetElectroCardioGram();
  if (ecg != nullptr)
    m_EquipmentSystems.push_back(ecg);
  SEInhaler* inh = (SEInhaler*)engine.GetInhaler();
  if (inh != nullptr)
    m_EquipmentSystems.push_back(inh);
  m_ForceConnection = false;
}

PhysiologyEngineTrack::PhysiologyEngineTrack(SEPatient& patient, SESubstanceManager& subMgr, SECompartmentManager& cmptMgr,
                                             const std::vector<SESystem*>& physiology, const std::vector<SESystem*>& equipment)
  : Loggable(patient.GetLogger())
  , m_DataRequestMgr(patient.GetLogger())
  , m_Patient(&patient)
  , m_SubMgr(&subMgr)
  , m_CmptMgr(&cmptMgr)
  , m_DataTrack(patient.GetLogger())
{
  for (auto* p : physiology) {
    m_PhysiologySystems.push_back(p);
  }
  for (auto* e : equipment) {
    m_EquipmentSystems.push_back(e);
  }
  m_ForceConnection = false;
}
PhysiologyEngineTrack::PhysiologyEngineTrack(PhysiologyEngineTrack&& obj)
  : m_ForceConnection(std::move(obj.m_ForceConnection))
  , m_DataTrack(std::move(obj.m_DataTrack))
  #ifndef ANDROID
  , m_ss(std::move(obj.m_ss))
  , m_ResultsStream(std::move(obj.m_ResultsStream))
  #endif
  , m_DataRequestMgr(std::move(obj.m_DataRequestMgr))
  , m_Patient(obj.m_Patient)
  , m_SubMgr(obj.m_SubMgr)
  , m_CmptMgr(obj.m_CmptMgr)
  , m_Environment(std::move(obj.m_Environment))
  , m_PhysiologySystems(std::move(obj.m_PhysiologySystems))
  , m_EquipmentSystems(std::move(obj.m_EquipmentSystems))
  , m_Request2Scalar(std::move(obj.m_Request2Scalar))
{
}
PhysiologyEngineTrack& PhysiologyEngineTrack::operator=(PhysiologyEngineTrack&& rhs)
{
  m_ForceConnection = std::move(rhs.m_ForceConnection);
  m_DataTrack = std::move(rhs.m_DataTrack);
  #ifndef ANDROID
  m_ss = std::move(rhs.m_ss);
  m_ResultsStream = std::move(rhs.m_ResultsStream);
  #else 
    //TODO: Reduce this condition to an NDK revision check. 
    //      THis very much breaks DataTracks on NDK builds.
  #endif
  m_DataRequestMgr = std::move(rhs.m_DataRequestMgr);
  m_Patient = rhs.m_Patient;
  m_SubMgr = rhs.m_SubMgr;
  m_CmptMgr = rhs.m_CmptMgr;
  m_Environment = std::move(rhs.m_Environment);
  m_PhysiologySystems = std::move(rhs.m_PhysiologySystems);
  m_EquipmentSystems = std::move(rhs.m_EquipmentSystems);
  m_Request2Scalar = std::move(rhs.m_Request2Scalar);
  return *this;
}
PhysiologyEngineTrack::~PhysiologyEngineTrack()
{
  Clear();
}

void PhysiologyEngineTrack::Clear()
{
  ResetFile();
  m_ForceConnection = false;
  DELETE_MAP_SECOND(m_Request2Scalar);
}

void PhysiologyEngineTrack::ResetFile()
{
  if (m_ResultsStream.is_open()) {
    m_ResultsStream.close();
  }
}

DataTrack& PhysiologyEngineTrack::GetDataTrack()
{
  return m_DataTrack;
}

SEDataRequestScalar* PhysiologyEngineTrack::GetScalar(SEDataRequest* dr)
{
  if (dr) {
    return m_Request2Scalar[dr];
  }
  return nullptr;
}


void PhysiologyEngineTrack::SetupRequests(bool append)
{
  bool isOpen = m_ResultsStream.is_open();
  if (!isOpen || m_ForceConnection) { // Process/Hook up all requests with their associated scalars
    for (SEDataRequest* dr : m_DataRequestMgr.GetDataRequests()) {
      if (!TrackRequest(*dr)) { // Could not hook this up, get rid of it
        m_ss << "Unable to find data for " << m_Request2Scalar[dr]->Heading;
        Error(m_ss);
      }
    }
    m_ForceConnection = false;
  }
  m_DataTrack.Probe("_XVER_", 0);
  m_DataTrack.SetFormatting("_XVER_", 6);

  m_DataTrack.Probe("m_HeartRateModifierSympathetic", 0);
  m_DataTrack.Probe("m_HeartRateModifierVagal", 0);
  m_DataTrack.Probe("m_VagalSignal_Hz", 0);
  m_DataTrack.Probe("m_SympatheticSinoatrialSignal_Hz", 0);
  m_DataTrack.Probe("m_AfferentBaroreceptorCarotid_Hz", 0);
  m_DataTrack.Probe("m_AfferentBaroreceptorAortic_Hz", 0);
  m_DataTrack.Probe("m_AfferentChemoreceptor_Hz", 0);
  m_DataTrack.Probe("m_AfferentPulmonaryStretchReceptor_Hz", 0);
  m_DataTrack.Probe("m_AfferentCardiopulmonary_Hz", 0);
  m_DataTrack.Probe("m_CarotidBaroreceptorStrain", 0);
  m_DataTrack.Probe("m_AorticBaroreceptorStrain", 0);
  m_DataTrack.Probe("m_ChemoreceptorFiringRateSetPoint_Hz", 0);
  m_DataTrack.Probe("m_CardiopulmonaryInputBaseline_mmHg", 0);
  m_DataTrack.Probe("m_CardiopulmonaryInput_mmHg", 0);
  m_DataTrack.Probe("m_MeanLungVolume_L", 0);
  m_DataTrack.Probe("m_BaroreceptorOperatingPoint_mmHg", 0);

  m_DataTrack.Probe("m_HeartOxygenBaseline", 0);
  m_DataTrack.Probe("m_MuscleOxygenBaseline", 0);
  m_DataTrack.Probe("m_OxygenAutoregulatorHeart", 0);
  m_DataTrack.Probe("m_OxygenAutoregulatorMuscle", 0);
  m_DataTrack.Probe("m_HypoxiaThresholdHeart", 0);
  m_DataTrack.Probe("m_HypoxiaThresholdPeripheral", 0);
  m_DataTrack.Probe("m_HypercapniaThresholdHeart", 0);
  m_DataTrack.Probe("m_HypercapniaThresholdPeripheral", 0);
  // m_DataTrack.Probe("metabolicFraction", 0);
  m_DataTrack.Probe("m_SympatheticPeripheralSignal_Hz", 0);
  m_DataTrack.Probe("m_SympatheticPeripheralSignalBaseline_Hz", 0);
  m_DataTrack.Probe("m_SympatheticPeripheralSignalFatigue", 0);
  m_DataTrack.Probe("m_SympatheticSinoatrialSignalBaseline_Hz", 0);
  m_DataTrack.Probe("m_ResistanceModifierMuscle", 0);

  m_DataTrack.Probe("m_OverrideHR_Conformant_Per_min", 0);
  m_DataTrack.Probe("m_CardiacCyclePeriod_s", 0);
  m_DataTrack.Probe("m_StartSystole", 0);
  m_DataTrack.Probe("m_SystoleCount", 0);
  m_DataTrack.Probe("m_SystoleCountTime", 0);
  m_DataTrack.Probe("m_CurrentCardiacCycleTime_s", 0);
  m_DataTrack.Probe("m_CurrentCardiacCycleDuration_s", 0);
  m_DataTrack.Probe("m_HeartFlowDetected", 0);
  m_DataTrack.Probe("m_LHeartFlow_ml_Per_s", 0);
  m_DataTrack.Probe("m_Set_HR_Per_Min_prev", 0);
  m_DataTrack.Probe("m_Set_HR_Per_Min", 0);
  m_DataTrack.Probe("m_LeftHeartElastanceModifier", 0);
  m_DataTrack.Probe("m_LeftHeartElastanceMin_mmHg_Per_mL", 0);
  m_DataTrack.Probe("m_LeftHeartElastanceMax_mmHg_Per_mL", 0);
  m_DataTrack.Probe("m_LeftHeartElastance_mmHg_Per_mL", 0);
  m_DataTrack.Probe("m_RightHeartElastanceMin_mmHg_Per_mL", 0);
  m_DataTrack.Probe("m_RightHeartElastanceMax_mmHg_Per_mL", 0);
  m_DataTrack.Probe("m_RightHeartElastance_mmHg_Per_mL", 0);
  m_DataTrack.Probe("m_pAortaToMuscle_R", 0);
  m_DataTrack.Probe("m_pAortaToMuscle_next_R", 0);
  m_DataTrack.Probe("m_pAortaToMuscle_baseline_R", 0);
  m_DataTrack.Probe("m_pAortaToMuscle_flow", 0);
  m_DataTrack.Probe("m_minIndividualSystemicResistance__mmHg_s_Per_mL", 0);

  m_DataTrack.Probe("m_has_exercise", 0);
  m_DataTrack.Probe("m_exercise_energy_inc_kcal_Per_day", 0);
  m_DataTrack.Probe("m_exerciseIntensity", 0);

  // BloodChem
  m_DataTrack.Probe("m_SID", 0);
  m_DataTrack.Probe("m_otherCations_mmol_Per_L", 0);
  m_DataTrack.Probe("vcNa", 0);
  m_DataTrack.Probe("vcK", 0);
  m_DataTrack.Probe("vcCl", 0);
  m_DataTrack.Probe("vcLac", 0);
  m_DataTrack.Probe("vcKetones", 0);

  m_DataTrack.SetFormatting("m_HeartRateModifierSympathetic", 6);
  m_DataTrack.SetFormatting("m_HeartRateModifierVagal", 6);
  m_DataTrack.SetFormatting("m_VagalSignal_Hz", 6);
  m_DataTrack.SetFormatting("m_SympatheticSinoatrialSignal_Hz", 6);
  m_DataTrack.SetFormatting("m_AfferentBaroreceptorCarotid_Hz", 6);
  m_DataTrack.SetFormatting("m_AfferentBaroreceptorAortic_Hz", 6);
  m_DataTrack.SetFormatting("m_AfferentChemoreceptor_Hz", 6);
  m_DataTrack.SetFormatting("m_AfferentPulmonaryStretchReceptor_Hz", 6);
  m_DataTrack.SetFormatting("m_AfferentCardiopulmonary_Hz", 6);
  m_DataTrack.SetFormatting("m_CarotidBaroreceptorStrain", 6);
  m_DataTrack.SetFormatting("m_AorticBaroreceptorStrain", 6);
  m_DataTrack.SetFormatting("m_ChemoreceptorFiringRateSetPoint_Hz", 6);
  m_DataTrack.SetFormatting("m_CardiopulmonaryInputBaseline_mmHg", 6);
  m_DataTrack.SetFormatting("m_CardiopulmonaryInput_mmHg", 6);
  m_DataTrack.SetFormatting("m_MeanLungVolume_L", 6);
  m_DataTrack.SetFormatting("m_BaroreceptorOperatingPoint_mmHg", 6);

  m_DataTrack.SetFormatting("m_HeartOxygenBaseline", 12);
  m_DataTrack.SetFormatting("m_MuscleOxygenBaseline", 12);
  m_DataTrack.SetFormatting("m_OxygenAutoregulatorHeart", 12);
  m_DataTrack.SetFormatting("m_OxygenAutoregulatorMuscle", 12);
  m_DataTrack.SetFormatting("m_HypoxiaThresholdHeart", 12);
  m_DataTrack.SetFormatting("m_HypoxiaThresholdPeripheral", 12);
  m_DataTrack.SetFormatting("m_HypercapniaThresholdHeart", 12);
  m_DataTrack.SetFormatting("m_HypercapniaThresholdPeripheral", 12);
  // m_DataTrack.SetFormatting("metabolicFraction", 12);
  m_DataTrack.SetFormatting("m_SympatheticPeripheralSignal_Hz", 12);
  m_DataTrack.SetFormatting("m_SympatheticPeripheralSignalBaseline_Hz", 12);
  m_DataTrack.SetFormatting("m_SympatheticPeripheralSignalFatigue", 12);
  m_DataTrack.SetFormatting("m_SympatheticSinoatrialSignalBaseline_Hz", 12);
  m_DataTrack.SetFormatting("m_ResistanceModifierMuscle", 12);

  m_DataTrack.SetFormatting("m_OverrideHR_Conformant_Per_min", 6);
  m_DataTrack.SetFormatting("m_CardiacCyclePeriod_s", 6);
  m_DataTrack.SetFormatting("m_StartSystole", 6);
  m_DataTrack.SetFormatting("m_SystoleCount", 6);
  m_DataTrack.SetFormatting("m_SystoleCountTime", 16);
  m_DataTrack.SetFormatting("m_CurrentCardiacCycleTime_s", 6);
  m_DataTrack.SetFormatting("m_CurrentCardiacCycleDuration_s", 6);
  m_DataTrack.SetFormatting("m_HeartFlowDetected", 6);
  m_DataTrack.SetFormatting("m_LHeartFlow_ml_Per_s", 6);
  m_DataTrack.SetFormatting("m_Set_HR_Per_Min_prev", 6);
  m_DataTrack.SetFormatting("m_Set_HR_Per_Min", 6);

  m_DataTrack.SetFormatting("m_LeftHeartElastanceModifier", 6);
  m_DataTrack.SetFormatting("m_LeftHeartElastanceMin_mmHg_Per_mL", 6);
  m_DataTrack.SetFormatting("m_LeftHeartElastanceMax_mmHg_Per_mL", 6);
  m_DataTrack.SetFormatting("m_LeftHeartElastance_mmHg_Per_mL", 6);
  m_DataTrack.SetFormatting("m_RightHeartElastanceMin_mmHg_Per_mL", 6);
  m_DataTrack.SetFormatting("m_RightHeartElastanceMax_mmHg_Per_mL", 6);
  m_DataTrack.SetFormatting("m_RightHeartElastance_mmHg_Per_mL", 6);
  m_DataTrack.SetFormatting("m_pAortaToMuscle_R", 12);
  m_DataTrack.SetFormatting("m_pAortaToMuscle_next_R", 12);
  m_DataTrack.SetFormatting("m_pAortaToMuscle_baseline_R", 12);
  m_DataTrack.SetFormatting("m_pAortaToMuscle_flow", 12);
  m_DataTrack.SetFormatting("m_minIndividualSystemicResistance__mmHg_s_Per_mL", 12);

  m_DataTrack.SetFormatting("m_has_exercise", 12);
  m_DataTrack.SetFormatting("m_exercise_energy_inc_kcal_Per_day", 12);
  m_DataTrack.SetFormatting("m_exerciseIntensity", 12);

  // BloodChem
  m_DataTrack.SetFormatting("m_SID", 12);
  m_DataTrack.SetFormatting("m_otherCations_mmol_Per_L", 12);
  m_DataTrack.SetFormatting("vcNa", 12);
  m_DataTrack.SetFormatting("vcK", 12);
  m_DataTrack.SetFormatting("vcCl", 12);
  m_DataTrack.SetFormatting("vcLac", 12);
  m_DataTrack.SetFormatting("vcKetones", 12);

  // Create the file now that all probes and requests have been added to the track
  // So we get columns for all of our data
  if (!isOpen) {
    m_DataTrack.CreateFile(m_DataRequestMgr.GetResultsFilename(), m_ResultsStream, (append) ? std::ios_base::app : std::ios_base::trunc);
  }
}

void PhysiologyEngineTrack::TrackData(double time_s, bool append)
{
  if (!m_DataRequestMgr.HasDataRequests()) {
    return; // Nothing to do here...
  }

  SetupRequests(append);
  PullData();
  m_DataTrack.StreamProbesToFile(time_s, m_ResultsStream);
}
void PhysiologyEngineTrack::PullData()
{
  SEDataRequestScalar* ds;
  for (SEDataRequest* dr : m_DataRequestMgr.GetDataRequests()) {
    ds = m_Request2Scalar[dr];
    if (ds == nullptr) {
      Error("You cannot modify CSV Results file data requests in the middle of a run.");
      Error(std::string { "Ignoring data request " } + dr->GetName());
      continue;
    }
    if (!ds->HasScalar()) {
      m_DataTrack.Probe(ds->Heading, SEScalar::dNaN());
      continue;
    }
    ds->UpdateScalar(); // Update compartment if needed
    if (ds->IsValid()) {
      if (ds->HasUnit()) {
        if (dr->GetUnit() == nullptr)
          dr->SetUnit(*ds->GetUnit());
        m_DataTrack.Probe(ds->Heading, ds->GetValue(dr->GetUnit()->GetString()));
      } else
        m_DataTrack.Probe(ds->Heading, ds->GetValue());
    } else if (ds->IsInfinity())
      m_DataTrack.Probe(ds->Heading, std::numeric_limits<double>::infinity());
    else
      m_DataTrack.Probe(ds->Heading, SEScalar::dNaN());
  }
  // SENervousSystem* n1 = (SENervousSystem*)m_PhysiologySystems[9];
  // const double x_ComplianceScale = n1->GetComplianceScale().GetValue();
  m_DataTrack.Probe("_XVER_", _XVER_);

  Nervous* n2 = (Nervous*)m_PhysiologySystems[9];
  m_DataTrack.Probe("m_HeartRateModifierSympathetic", n2->m_HeartRateModifierSympathetic);
  m_DataTrack.Probe("m_HeartRateModifierVagal", n2->m_HeartRateModifierVagal);
  m_DataTrack.Probe("m_VagalSignal_Hz", n2->m_VagalSignal_Hz);
  m_DataTrack.Probe("m_SympatheticSinoatrialSignal_Hz", n2->m_SympatheticSinoatrialSignal_Hz);
  m_DataTrack.Probe("m_AfferentBaroreceptorCarotid_Hz", n2-> m_AfferentBaroreceptorCarotid_Hz);
  m_DataTrack.Probe("m_AfferentBaroreceptorAortic_Hz", n2->m_AfferentBaroreceptorAortic_Hz);
  m_DataTrack.Probe("m_AfferentChemoreceptor_Hz", n2->m_AfferentChemoreceptor_Hz);
  m_DataTrack.Probe("m_AfferentPulmonaryStretchReceptor_Hz", n2->m_AfferentPulmonaryStretchReceptor_Hz);
  m_DataTrack.Probe("m_AfferentCardiopulmonary_Hz", n2->m_AfferentCardiopulmonary_Hz);
  m_DataTrack.Probe("m_CarotidBaroreceptorStrain", n2->m_CarotidBaroreceptorStrain);
  m_DataTrack.Probe("m_AorticBaroreceptorStrain", n2->m_AorticBaroreceptorStrain);
  m_DataTrack.Probe("m_ChemoreceptorFiringRateSetPoint_Hz", n2->m_ChemoreceptorFiringRateSetPoint_Hz);
  m_DataTrack.Probe("m_CardiopulmonaryInputBaseline_mmHg", n2->m_CardiopulmonaryInputBaseline_mmHg);
  m_DataTrack.Probe("m_CardiopulmonaryInput_mmHg", n2->m_CardiopulmonaryInput_mmHg);
  m_DataTrack.Probe("m_MeanLungVolume_L", n2->m_MeanLungVolume_L);
  m_DataTrack.Probe("m_BaroreceptorOperatingPoint_mmHg", n2->m_BaroreceptorOperatingPoint_mmHg);
  m_DataTrack.Probe("m_HeartOxygenBaseline", n2->m_HeartOxygenBaseline);
  m_DataTrack.Probe("m_MuscleOxygenBaseline", n2->m_MuscleOxygenBaseline);
  m_DataTrack.Probe("m_OxygenAutoregulatorHeart", n2->m_OxygenAutoregulatorHeart);
  m_DataTrack.Probe("m_OxygenAutoregulatorMuscle", n2->m_OxygenAutoregulatorMuscle);
  m_DataTrack.Probe("m_HypoxiaThresholdHeart", n2->m_HypoxiaThresholdHeart);
  m_DataTrack.Probe("m_HypoxiaThresholdPeripheral", n2->m_HypoxiaThresholdPeripheral);
  m_DataTrack.Probe("m_HypercapniaThresholdHeart", n2->m_HypercapniaThresholdHeart);
  m_DataTrack.Probe("m_HypercapniaThresholdPeripheral", n2->m_HypercapniaThresholdPeripheral);
  // m_DataTrack.Probe("metabolicFraction", n2->m_data.GetEnergy().GetTotalMetabolicRate(PowerUnit::W) / n2->m_data.GetPatient().GetBasalMetabolicRate(PowerUnit::W));
  m_DataTrack.Probe("m_SympatheticPeripheralSignal_Hz",         n2->m_SympatheticPeripheralSignal_Hz);
  m_DataTrack.Probe("m_SympatheticPeripheralSignalBaseline_Hz", n2->m_SympatheticPeripheralSignalBaseline_Hz);
  m_DataTrack.Probe("m_SympatheticPeripheralSignalFatigue",     n2->m_SympatheticPeripheralSignalFatigue);
  m_DataTrack.Probe("m_SympatheticSinoatrialSignalBaseline_Hz", n2->m_SympatheticSinoatrialSignalBaseline_Hz);
  m_DataTrack.Probe("m_ResistanceModifierMuscle",               n2->m_ResistanceModifierMuscle);

  Cardiovascular* c2 = (Cardiovascular*)m_PhysiologySystems[1];
  m_DataTrack.Probe("m_OverrideHR_Conformant_Per_min", c2->m_OverrideHR_Conformant_Per_min);
  m_DataTrack.Probe("m_CardiacCyclePeriod_s", c2->m_CardiacCyclePeriod_s);
  m_DataTrack.Probe("m_StartSystole", c2->m_StartSystole ? 1.0 : 0.0);
  m_DataTrack.Probe("m_SystoleCount", c2->m_SystoleCount);
  m_DataTrack.Probe("m_SystoleCountTime", c2->m_SystoleCountTime);
  m_DataTrack.Probe("m_CurrentCardiacCycleTime_s", c2->m_CurrentCardiacCycleTime_s);
  m_DataTrack.Probe("m_CurrentCardiacCycleDuration_s", c2->m_CurrentCardiacCycleDuration_s);
  m_DataTrack.Probe("m_HeartFlowDetected", c2->m_HeartFlowDetected ? 1.0 : 0.0);
  m_DataTrack.Probe("m_LHeartFlow_ml_Per_s", c2->m_LeftHeartToAorta->GetNextFlow(VolumePerTimeUnit::mL_Per_s));
  m_DataTrack.Probe("m_Set_HR_Per_Min_prev", c2->m_Set_HR_Per_Min_prev);
  m_DataTrack.Probe("m_Set_HR_Per_Min", c2->m_Set_HR_Per_Min);
  m_DataTrack.Probe("m_LeftHeartElastanceModifier", c2->m_LeftHeartElastanceModifier);
  m_DataTrack.Probe("m_LeftHeartElastanceMin_mmHg_Per_mL", c2->m_LeftHeartElastanceMin_mmHg_Per_mL);
  m_DataTrack.Probe("m_LeftHeartElastanceMax_mmHg_Per_mL", c2->m_LeftHeartElastanceMax_mmHg_Per_mL);
  m_DataTrack.Probe("m_LeftHeartElastance_mmHg_Per_mL", c2->m_LeftHeartElastance_mmHg_Per_mL);
  m_DataTrack.Probe("m_RightHeartElastanceMin_mmHg_Per_mL", c2->m_RightHeartElastanceMin_mmHg_Per_mL);
  m_DataTrack.Probe("m_RightHeartElastanceMax_mmHg_Per_mL", c2->m_RightHeartElastanceMax_mmHg_Per_mL);
  m_DataTrack.Probe("m_RightHeartElastance_mmHg_Per_mL", c2->m_RightHeartElastance_mmHg_Per_mL);
  m_DataTrack.Probe("m_pAortaToMuscle_R", c2->m_pAortaToMuscle->GetResistance(FlowResistanceUnit::mmHg_s_Per_mL));
  m_DataTrack.Probe("m_pAortaToMuscle_next_R", c2->m_pAortaToMuscle->GetNextResistance(FlowResistanceUnit::mmHg_s_Per_mL));
  m_DataTrack.Probe("m_pAortaToMuscle_baseline_R", c2->m_pAortaToMuscle->GetResistanceBaseline(FlowResistanceUnit::mmHg_s_Per_mL));
  m_DataTrack.Probe("m_pAortaToMuscle_flow", c2->m_pAortaToMuscle->GetNextFlow(VolumePerTimeUnit::mL_Per_s));
  m_DataTrack.Probe("m_minIndividualSystemicResistance__mmHg_s_Per_mL", c2->m_minIndividialSystemicResistance__mmHg_s_Per_mL);

  Energy* e2 = (Energy*)m_PhysiologySystems[3];
  m_DataTrack.Probe("m_has_exercise", e2->m_has_exercise);
  m_DataTrack.Probe("m_exercise_energy_inc_kcal_Per_day", e2->m_exercise_energy_inc_kcal_Per_day);
  m_DataTrack.Probe("m_exerciseIntensity", e2->m_exerciseIntensity);

  BloodChemistry* bc2 = (BloodChemistry*)m_PhysiologySystems[0];
  m_DataTrack.Probe("m_SID", bc2->m_SID);
  m_DataTrack.Probe("m_otherCations_mmol_Per_L", bc2->m_otherCations_mmol_Per_L);
  m_DataTrack.Probe("vcNa", bc2->m_venaCavaSodium->GetMolarity(AmountPerVolumeUnit::mmol_Per_L));
  m_DataTrack.Probe("vcK", bc2->m_venaCavaPotassium->GetMolarity(AmountPerVolumeUnit::mmol_Per_L));
  m_DataTrack.Probe("vcCl", bc2->m_venaCavaChloride->GetMolarity(AmountPerVolumeUnit::mmol_Per_L));
  m_DataTrack.Probe("vcLac", bc2->m_venaCavaLactate->GetMolarity(AmountPerVolumeUnit::mmol_Per_L));
  m_DataTrack.Probe("vcKetones", bc2->m_venaCavaKetones->GetMolarity(AmountPerVolumeUnit::mmol_Per_L));
}

bool PhysiologyEngineTrack::TrackRequest(SEDataRequest& dr)
{
  std::string name = dr.GetName();
  SEDataRequestScalar* ds = new SEDataRequestScalar(GetLogger());
  m_Request2Scalar[&dr] = ds;

  bool success = ConnectRequest(dr, *ds);

  // Now build out the heading string
  if (dynamic_cast<const SEPhysiologyDataRequest*>(&dr) != nullptr || dynamic_cast<const SEEquipmentDataRequest*>(&dr) != nullptr || dynamic_cast<const SEEnvironmentDataRequest*>(&dr) != nullptr || dynamic_cast<const SEPatientDataRequest*>(&dr) != nullptr) {
    if (dynamic_cast<const SEPatientDataRequest*>(&dr) != nullptr)
      m_ss << "Patient";

    if (!dr.GetUnit())
      m_ss << dr.GetName();
    else
      m_ss << dr.GetName() << "(" << *dr.GetUnit() << ")";

    ds->Heading = Space2Underscore(m_ss.str());
    m_ss.str(""); //Reset Buffer
    m_DataTrack.Probe(ds->Heading, 0);
    m_DataTrack.SetFormatting(ds->Heading, dr);
    return success;
  }
  const SEGasCompartmentDataRequest* gasDR = dynamic_cast<const SEGasCompartmentDataRequest*>(&dr);
  if (gasDR != nullptr) { // Not including type in the name, as I am making the assumption you don't have compartments with the same name even between types
    if (gasDR->HasSubstance()) {
      if (!dr.GetUnit())
        m_ss << gasDR->GetCompartment() << "-" << gasDR->GetSubstance()->GetName() << "-" << gasDR->GetName();
      else
        m_ss << gasDR->GetCompartment() << "-" << gasDR->GetSubstance()->GetName() << "-" << gasDR->GetName() << "(" << *gasDR->GetUnit() << ")";
    } else {
      if (!dr.GetUnit())
        m_ss << gasDR->GetCompartment() << "-" << gasDR->GetName();
      else
        m_ss << gasDR->GetCompartment() << "-" << gasDR->GetName() << "(" << *gasDR->GetUnit() << ")";
    }
    ds->Heading = Space2Underscore(m_ss.str());
    m_ss.str(""); //Reset Buffer
    m_DataTrack.Probe(ds->Heading, 0);
    m_DataTrack.SetFormatting(ds->Heading, dr);
    return success;
  }
  const SELiquidCompartmentDataRequest* liquidDR = dynamic_cast<const SELiquidCompartmentDataRequest*>(&dr);
  if (liquidDR != nullptr) { // Not including type in the name, as I am making the assumption you don't have compartments with the same name even between types
    if (liquidDR->HasSubstance()) {
      if (!dr.GetUnit())
        m_ss << liquidDR->GetCompartment() << "-" << liquidDR->GetSubstance()->GetName() << "-" << liquidDR->GetName();
      else
        m_ss << liquidDR->GetCompartment() << "-" << liquidDR->GetSubstance()->GetName() << "-" << liquidDR->GetName() << "(" << *liquidDR->GetUnit() << ")";
    } else {
      if (!dr.GetUnit())
        m_ss << liquidDR->GetCompartment() << "-" << liquidDR->GetName();
      else
        m_ss << liquidDR->GetCompartment() << "-" << liquidDR->GetName() << "(" << *liquidDR->GetUnit() << ")";
    }
    ds->Heading = Space2Underscore(m_ss.str());
    m_ss.str(""); //Reset Buffer
    m_DataTrack.Probe(ds->Heading, 0);
    m_DataTrack.SetFormatting(ds->Heading, dr);
    return success;
  }
  const SEThermalCompartmentDataRequest* thermalDR = dynamic_cast<const SEThermalCompartmentDataRequest*>(&dr);
  if (thermalDR != nullptr) { // Not including type in the name, as I am making the assumption you don't have compartments with the same name even between types

    if (!dr.GetUnit())
      m_ss << thermalDR->GetCompartment() << "-" << thermalDR->GetName();
    else
      m_ss << thermalDR->GetCompartment() << "-" << thermalDR->GetName() << "(" << *thermalDR->GetUnit() << ")";

    ds->Heading = Space2Underscore(m_ss.str());
    m_ss.str(""); //Reset Buffer
    m_DataTrack.Probe(ds->Heading, 0);
    m_DataTrack.SetFormatting(ds->Heading, dr);
    return success;
  }
  const SETissueCompartmentDataRequest* tissueDR = dynamic_cast<const SETissueCompartmentDataRequest*>(&dr);
  if (tissueDR != nullptr) {
    if (!dr.GetUnit())
      m_ss << tissueDR->GetCompartment() << "-" << tissueDR->GetName();
    else
      m_ss << tissueDR->GetCompartment() << "-" << tissueDR->GetName() << "(" << *tissueDR->GetUnit() << ")";

    ds->Heading = Space2Underscore(m_ss.str());
    m_ss.str(""); //Reset Buffer
    m_DataTrack.Probe(ds->Heading, 0);
    m_DataTrack.SetFormatting(ds->Heading, dr);
    return success;
  }
  const SESubstanceDataRequest* subDR = dynamic_cast<const SESubstanceDataRequest*>(&dr);
  if (subDR != nullptr) {
    if (subDR->HasCompartment()) {
      if (!dr.GetUnit())
        m_ss << subDR->GetSubstance()->GetName() << "-" << subDR->GetCompartment() << "-" << subDR->GetName();
      else
        m_ss << subDR->GetSubstance()->GetName() << "-" << subDR->GetCompartment() << "-" << subDR->GetName() << "(" << *subDR->GetUnit() << ")";
      ds->Heading = Space2Underscore(m_ss.str());
      m_ss.str(""); //Reset Buffer
      m_DataTrack.Probe(ds->Heading, 0);
      m_DataTrack.SetFormatting(ds->Heading, dr);
      return success;
    } else {
      if (!dr.GetUnit())
        m_ss << subDR->GetSubstance()->GetName() << "-" << subDR->GetName();
      else
        m_ss << subDR->GetSubstance()->GetName() << "-" << subDR->GetName() << "(" << *subDR->GetUnit() << ")";
      ds->Heading = Space2Underscore(m_ss.str());
      m_ss.str(""); //Reset Buffer
      m_DataTrack.Probe(ds->Heading, 0);
      m_DataTrack.SetFormatting(ds->Heading, dr);
      return success;
    }
  }
  m_ss << "Unhandled data request : " << name << std::endl;
  Fatal(m_ss);
  return false;
}

bool PhysiologyEngineTrack::ConnectRequest(SEDataRequest& dr, SEDataRequestScalar& ds)
{
  std::string name = dr.GetName();
  if (dynamic_cast<const SEPhysiologyDataRequest*>(&dr) != nullptr) {
    // Make sure we mapped something
    ds.SetScalar(SESystem::GetScalar(name, &m_PhysiologySystems), dr);
    return true;
  }
  if (dynamic_cast<const SEEnvironmentDataRequest*>(&dr) != nullptr) {
    // Make sure we mapped something
    ds.SetScalar(m_Environment->GetScalar(name), dr);
    return true;
  }
  if (dynamic_cast<const SEEquipmentDataRequest*>(&dr) != nullptr) {
    ds.SetScalar(SESystem::GetScalar(name, &m_EquipmentSystems), dr);
    return true;
  }
  const SEGasCompartmentDataRequest* gasDR = dynamic_cast<const SEGasCompartmentDataRequest*>(&dr);
  if (gasDR != nullptr) {
    if (!m_CmptMgr->HasGasCompartment(gasDR->GetCompartment())) {
      Error(std::string { "Unknown gas compartment : " } + gasDR->GetCompartment());
      return false;
    }
    // Removing const because I need to create objects in order to track those objects
    SEGasCompartment* gasCmpt = (SEGasCompartment*)m_CmptMgr->GetGasCompartment(gasDR->GetCompartment());

    if (gasDR->HasSubstance()) {
      // Activate this substance so compartments have it
      m_SubMgr->AddActiveSubstance(*gasDR->GetSubstance());
      if (gasCmpt->HasChildren()) {
        if (name == "Volume")
          ds.UpdateProperty = CompartmentUpdate::Volume;
        else if (name == "VolumeFraction")
          ds.UpdateProperty = CompartmentUpdate::VolumeFraction;
        else if (name == "PartialPressure")
          ds.UpdateProperty = CompartmentUpdate::PartialPressure;
        ds.GasSubstance = gasCmpt->GetSubstanceQuantity(*gasDR->GetSubstance());
      }
      ds.SetScalar(gasCmpt->GetSubstanceQuantity(*gasDR->GetSubstance())->GetScalar(name), dr);
    } else {
      if (gasCmpt->HasChildren() || gasCmpt->HasNodeMapping()) {
        if (name == "Volume")
          ds.UpdateProperty = CompartmentUpdate::Volume;
        if (name == "Pressure")
          ds.UpdateProperty = CompartmentUpdate::Pressure;
      }

      { // Always Update these
        if (name == "InFlow")
          ds.UpdateProperty = CompartmentUpdate::InFlow;
        else if (name == "OutFlow")
          ds.UpdateProperty = CompartmentUpdate::OutFlow;
      }
      ds.GasCmpt = gasCmpt;
      ds.SetScalar(gasCmpt->GetScalar(name), dr);
    }
    return true;
  }
  const SELiquidCompartmentDataRequest* liquidDR = dynamic_cast<const SELiquidCompartmentDataRequest*>(&dr);
  if (liquidDR != nullptr) {
    if (!m_CmptMgr->HasLiquidCompartment(liquidDR->GetCompartment())) {
      Error(std::string { "Unknown liquid compartment : " } + liquidDR->GetCompartment());
      return false;
    }
    // Removing const because I need to create objects in order to track those objects
    SELiquidCompartment* liquidCmpt = (SELiquidCompartment*)m_CmptMgr->GetLiquidCompartment(liquidDR->GetCompartment());

    if (liquidDR->HasSubstance()) {
      // Activate this substance so compartments have it
      m_SubMgr->AddActiveSubstance(*liquidDR->GetSubstance());
      if (liquidCmpt->HasChildren()) {
        if (name == "Mass")
          ds.UpdateProperty = CompartmentUpdate::Mass;
        else if (name == "Concentration")
          ds.UpdateProperty = CompartmentUpdate::Concentration;
        else if (name == "Molarity")
          ds.UpdateProperty = CompartmentUpdate::Molarity;
        else if (name == "PartialPressure")
          ds.UpdateProperty = CompartmentUpdate::PartialPressure;
        else if (name == "Saturation")
          ds.UpdateProperty = CompartmentUpdate::Saturation;
        ds.LiquidSubstance = liquidCmpt->GetSubstanceQuantity(*liquidDR->GetSubstance());
      }
      ds.SetScalar(liquidCmpt->GetSubstanceQuantity(*liquidDR->GetSubstance())->GetScalar(name), dr);
    } else {
      if (liquidCmpt->HasChildren() || liquidCmpt->HasNodeMapping()) {
        if (name == "Volume")
          ds.UpdateProperty = CompartmentUpdate::Volume;
        if (name == "Pressure")
          ds.UpdateProperty = CompartmentUpdate::Pressure;
      }

      { // Always Update these
        if (name == "InFlow")
          ds.UpdateProperty = CompartmentUpdate::InFlow;
        else if (name == "OutFlow")
          ds.UpdateProperty = CompartmentUpdate::OutFlow;
      }
      ds.LiquidCmpt = liquidCmpt;
      ds.SetScalar(liquidCmpt->GetScalar(name), dr);
    }
    return true;
  }
  const SEThermalCompartmentDataRequest* thermalDR = dynamic_cast<const SEThermalCompartmentDataRequest*>(&dr);
  if (thermalDR != nullptr) {
    if (!m_CmptMgr->HasThermalCompartment(thermalDR->GetCompartment())) {
      Error(std::string { "Unknown thermal compartment : " } + thermalDR->GetCompartment());
      return false;
    }
    // Removing const because I need to create objects in order to track those objects
    SEThermalCompartment* thermalCmpt = (SEThermalCompartment*)m_CmptMgr->GetThermalCompartment(thermalDR->GetCompartment());

    if (thermalCmpt->HasChildren() || thermalCmpt->HasNodeMapping()) {
      if (name == "Heat")
        ds.UpdateProperty = CompartmentUpdate::Heat;
      if (name == "Temperature")
        ds.UpdateProperty = CompartmentUpdate::Temperature;
    }

    { // Always Update these
      if (name == "HeatTransferRateIn")
        ds.UpdateProperty = CompartmentUpdate::HeatTransferRateIn;
      else if (name == "HeatTransferRateOut")
        ds.UpdateProperty = CompartmentUpdate::HeatTransferRateOut;
    }
    ds.ThermalCmpt = thermalCmpt;
    ds.SetScalar(thermalCmpt->GetScalar(name), dr);
    return true;
  }
  const SETissueCompartmentDataRequest* tissueDR = dynamic_cast<const SETissueCompartmentDataRequest*>(&dr);
  if (tissueDR != nullptr) {
    if (!m_CmptMgr->HasTissueCompartment(tissueDR->GetCompartment())) {
      Error(std::string { "Unknown tissue compartment : " } + tissueDR->GetCompartment());
      return false;
    }
    // Removing const because I need to create objects in order to track those objects
    SETissueCompartment* tissueCmpt = (SETissueCompartment*)m_CmptMgr->GetTissueCompartment(tissueDR->GetCompartment());
    ds.SetScalar(tissueCmpt->GetScalar(name), dr);
    return true;
  }
  if (dynamic_cast<const SEPatientDataRequest*>(&dr) != nullptr) {
    // casting of the const to modify the patient
    ds.SetScalar(m_Patient->GetScalar(name), dr);
    return true;
  }
  const SESubstanceDataRequest* subDR = dynamic_cast<const SESubstanceDataRequest*>(&dr);
  if (subDR != nullptr) {
    // Removing const because I want to allocate and grab scalars to track for later
    SESubstance* sub = (SESubstance*)subDR->GetSubstance();
    m_SubMgr->AddActiveSubstance(*sub);
    if (subDR->HasCompartment()) { // I don't really have a generic/reflexive way of doing this...yet
      if (subDR->GetName() == "PartitionCoefficient") {
        SESubstanceTissuePharmacokinetics& tk = sub->GetPK().GetTissueKinetics(subDR->GetCompartment());
        ds.SetScalar(&tk.GetPartitionCoefficient(), dr);
        return true;
      }
    } else {
      ds.SetScalar(sub->GetScalar(name), dr);
      return true;
    }
  }
  m_ss << "Unhandled data request : " << name << std::endl;
  Fatal(m_ss);
  return false;
}

void SEDataRequestScalar::SetScalar(const SEScalar* s, SEDataRequest& dr)
{
  if (s == nullptr) {
    Fatal(std::string { "Unknown Data Request : " } + dr.GetName());
    return;
  }
  SEGenericScalar::SetScalar(*s);
  if (HasUnit()) {
    if (!dr.HasRequestedUnit()) // Use set unit if none provide
      dr.SetUnit(*GetUnit());
    else {
      const CCompoundUnit* unit = GetCompoundUnit(dr.GetRequestedUnit());
      if (unit == nullptr) {
        std::stringstream ss;
        ss << dr.GetRequestedUnit() << " is not compatible with " << dr.GetName();
        Fatal(ss);
      }
      dr.SetUnit(*unit);
    }
  }
}

std::string SEDataRequestScalar::ToString() const
{
  if (HasScalar()) {
    return m_Scalar->ToString();
  } else if (HasUnit()) {
    return m_UnitScalar->ToString();
  }
  return "";
};

void SEDataRequestScalar::UpdateScalar()
{
  if (UpdateProperty == CompartmentUpdate::None)
    return;

  if (GasCmpt != nullptr) {
    switch (UpdateProperty) {
    case CompartmentUpdate::InFlow:
      GasCmpt->GetInFlow();
      return;
    case CompartmentUpdate::OutFlow:
      GasCmpt->GetOutFlow();
      return;
    case CompartmentUpdate::Volume:
      GasCmpt->GetVolume();
      return;
    case CompartmentUpdate::Pressure:
      GasCmpt->GetPressure();
      return;
    default:
      Error("Property is not supported on Gas Compartment");
    }
  } else if (GasSubstance != nullptr) {
    switch (UpdateProperty) {
    case CompartmentUpdate::Volume:
      GasSubstance->GetVolume();
      return;
    case CompartmentUpdate::VolumeFraction:
      GasSubstance->GetVolumeFraction();
      return;
    case CompartmentUpdate::PartialPressure:
      GasSubstance->GetPartialPressure();
      return;
    default:
      Error("Property is not supported on Gas Substance");
    }
  } else if (LiquidCmpt != nullptr) {
    switch (UpdateProperty) {
    case CompartmentUpdate::InFlow:
      LiquidCmpt->GetInFlow();
      return;
    case CompartmentUpdate::OutFlow:
      LiquidCmpt->GetOutFlow();
      return;
    case CompartmentUpdate::Volume:
      LiquidCmpt->GetVolume();
      return;
    case CompartmentUpdate::pH:
      LiquidCmpt->GetPH();
      return;
    case CompartmentUpdate::Pressure:
      LiquidCmpt->GetPressure();
      return;
    default:
      Error("Property is not supported on liquid Compartment");
    }
  } else if (LiquidSubstance != nullptr) {
    switch (UpdateProperty) {
    case CompartmentUpdate::Mass:
      LiquidSubstance->GetMass();
      return;
    case CompartmentUpdate::Molarity:
      LiquidSubstance->GetMolarity();
      return;
    case CompartmentUpdate::Concentration:
      LiquidSubstance->GetConcentration();
      return;
    case CompartmentUpdate::PartialPressure:
      LiquidSubstance->GetPartialPressure();
      return;
    case CompartmentUpdate::Saturation:
      LiquidSubstance->GetSaturation();
      return;
    default:
      Error("Property is not supported on Liquid Substance");
    }
  } else if (ThermalCmpt != nullptr) {
    switch (UpdateProperty) {
    case CompartmentUpdate::HeatTransferRateIn:
      ThermalCmpt->GetHeatTransferRateIn();
      return;
    case CompartmentUpdate::HeatTransferRateOut:
      ThermalCmpt->GetHeatTransferRateOut();
      return;
    case CompartmentUpdate::Heat:
      ThermalCmpt->GetHeat();
      return;
    case CompartmentUpdate::Temperature:
      ThermalCmpt->GetTemperature();
      return;
    default:
      Error("Property is not supported on Thermal Substance");
    }
  }
  Error("Could not update " + Heading);
}
}
