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

#pragma once
#include <biogears/cdm/patient/actions/SEPatientAction.h>
#include <biogears/schema/cdm/PatientActions.hxx>

namespace biogears {
namespace io {
  class PatientActions;
}
class BIOGEARS_API SEIntubation : public SEPatientAction {
  friend io::PatientActions;

public:
  SEIntubation();
  virtual ~SEIntubation() override;

  static constexpr const char* TypeTag() { return "SEIntubation"; };
  const char* classname() const override { return TypeTag(); }

  virtual void Clear() override; //clear memory

  virtual bool IsValid() const override;
  
  virtual bool IsActive() const override;
  

  virtual bool Load(const CDM::IntubationData& in);
  virtual CDM::IntubationData* Unload() const override;
  
  virtual CDM::enumIntubationType::value GetType() const;
  virtual void SetType(CDM::enumIntubationType::value t);
  virtual bool HasType() const;
  virtual void InvalidateType();

  virtual void ToString(std::ostream& str) const override;
          
  bool operator==( const SEIntubation& rhs) const;
  bool operator!=( const SEIntubation& rhs) const;

protected:
  virtual void Unload(CDM::IntubationData& data) const;

protected:
  CDM::enumIntubationType::value m_Type;
};
}