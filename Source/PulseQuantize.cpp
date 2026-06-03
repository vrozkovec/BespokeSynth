/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2026 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
/*
  ==============================================================================

    PulseQuantize.cpp
    Created: 3 Jun 2026
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PulseQuantize.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"

PulseQuantize::PulseQuantize()
{
}

PulseQuantize::~PulseQuantize()
{
   TheTransport->RemoveListener(this);
}

void PulseQuantize::Init()
{
   IDrawableModule::Init();

   mTransportListenerInfo = TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), true);
}

void PulseQuantize::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   DROPDOWN(mIntervalSelector, "interval", (int*)(&mInterval), 60);
   CHECKBOX(mArmedCheckbox, "armed", &mArmed);
   ENDUIBLOCK(mWidth, mHeight);

   mIntervalSelector->AddLabel("4", kInterval_4);
   mIntervalSelector->AddLabel("2", kInterval_2);
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
}

void PulseQuantize::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mIntervalSelector->Draw();
   mArmedCheckbox->Draw();
}

void PulseQuantize::Arm(float velocity, int flags)
{
   mPendingVelocity = velocity;
   mPendingFlags = flags;
   mArmed = true;
}

void PulseQuantize::OnPulse(double time, float velocity, int flags)
{
   ComputeSliders(0);

   if (!mEnabled)
   {
      DispatchPulse(GetPatchCableSource(), time, velocity, flags);
      return;
   }

   Arm(velocity, flags);
}

void PulseQuantize::OnTimeEvent(double time)
{
   if (!mEnabled)
      return;

   if (mArmed)
   {
      mArmed = false;
      DispatchPulse(GetPatchCableSource(), time, mPendingVelocity, mPendingFlags);
   }
}

void PulseQuantize::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mArmedCheckbox)
   {
      //the checkbox toggles mArmed before this is called; if the user (or a mapped midi button)
      //turned it on, arm a default pulse. turning it off simply cancels the queued pulse.
      if (mArmed)
         Arm(1, 0);
   }
}

void PulseQuantize::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mIntervalSelector)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
         transportListenerInfo->mInterval = mInterval;
   }
}

void PulseQuantize::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void PulseQuantize::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
