/*
* Copyright 2009, 2010, The Pink Petal Development Team.
* The Pink Petal Devloment Team are defined as the game's coders
* who meet on http://pinkpetal.org     // old site: http://pinkpetal .co.cc
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma region //	Includes and Externs			//
#include "cJobManager.h"
#include "cBrothel.h"
#include "cCentre.h"
#include "cCustomers.h"
#include "cRng.h"
#include "cInventory.h"
#include "sConfig.h"
#include "cRival.h"
#include <sstream>
#include "CLog.h"
#include "cTrainable.h"
#include "cTariff.h"
#include "cGold.h"
#include "cGangs.h"
#include "cMessageBox.h"
#include "libintl.h"

extern cRng g_Dice;
extern CLog g_LogFile;
extern cCustomers g_Customers;
extern cInventory g_InvManager;
extern cBrothelManager g_Brothels;
extern cCentreManager g_Centre;
extern cGangManager g_Gangs;
extern cMessageQue g_MessageQue;

#pragma endregion

// `J` Job House - Training - Full_Time_Job
bool cJobManager::WorkSOLesbian(sGirl* girl, sBrothel* brothel, bool Day0Night1, string& summary)
{
#pragma region //	Job setup				//
	stringstream ss; string girlName = girl->m_Realname;
	int actiontype = ACTION_WORKTRAINING;
	// if she was not in JOB_SO_LESBIAN yesterday, reset working days to 0 before proceding
	if (girl->m_YesterDayJob != JOB_SO_LESBIAN) girl->m_PrevWorkingDay = girl->m_WorkingDay = 0;
	if (girl->m_WorkingDay < 0) girl->m_WorkingDay = 0;
	girl->m_DayJob = girl->m_NightJob = JOB_SO_LESBIAN;	// it is a full time job

	if (g_Girls.HasTrait(girl, "Lesbian"))
	{
		ss << girlName << " is already a Lesbian.";
		if (Day0Night1 == SHIFT_DAY)	girl->m_Events.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_WARNING);
		girl->m_PrevDayJob = girl->m_PrevNightJob = girl->m_YesterDayJob = girl->m_YesterNightJob = girl->m_DayJob = girl->m_NightJob = JOB_HOUSEREST;
		girl->m_PrevWorkingDay = girl->m_WorkingDay = 0;
		return false;	// not refusing
	}

	ss << "You procede to change " << girlName << "'s sexual orientation to Lesbian.\n\n";

	g_Girls.UnequipCombat(girl);	// not for patient

	int enjoy = 0, wages = 10, tips = 0;
	int libido = 0;
	int msgtype = Day0Night1, imagetype = IMGTYPE_LESBIAN;

	// Base adjustment
	girl->m_WorkingDay += 10 + g_Dice % 11;
	// Positive Stats/Skills
	girl->m_WorkingDay += girl->lesbian() / 5;
	girl->m_WorkingDay += girl->group() / 20;
	girl->m_WorkingDay += girl->oralsex() / 25;
	girl->m_WorkingDay += girl->tittysex() / 50;
	girl->m_WorkingDay += girl->anal() / 50;
	girl->m_WorkingDay += girl->obedience() / 20;
	if (girl->pcfear() > 50)				girl->m_WorkingDay += g_Dice % (girl->pcfear() / 20);		// She will do as she is told
	if (girl->pclove() > 50)				girl->m_WorkingDay += g_Dice % (girl->pclove() / 20);		// She will do what you ask
	// Negative Stats/Skills
	girl->m_WorkingDay -= girl->spirit() / 25;
	if (girl->pchate() > 30)				girl->m_WorkingDay -= g_Dice % (girl->pchate() / 10);		// She will not do what you want
	if (girl->happiness() < 50)				girl->m_WorkingDay -= 1 + g_Dice % 5;						// She is not feeling like it
	if (girl->health() < 50)				girl->m_WorkingDay -= 1 + g_Dice % 5;						// She is feeling sick
	if (girl->tiredness() > 50)				girl->m_WorkingDay -= 1 + g_Dice % 5;						// She is tired
	if (girl->has_trait("Straight"))
	{
		ss << "Being used to working with something long and hard, she wasn't really sure what she was doing with her partner.\n";
		girl->m_WorkingDay -= girl->normalsex() / 5;				// it is hard to change something you are good at
	}
	if (girl->has_trait("Bisexual"))		girl->m_WorkingDay -= girl->normalsex() / 20;					// it is hard to change something you are good at
	// Positive Traits
	if (g_Girls.CheckVirginity(girl))		girl->m_WorkingDay += g_Dice.bell(2, 10);	// If she has not been with a man, what could she be missing
	if (girl->has_trait("Your Daughter"))	girl->m_WorkingDay += g_Dice.bell(2, 10);	// She sees you enjoying yourself with the ladies, lead by example?
	if (girl->has_trait("Futanari"))		girl->m_WorkingDay += 2;					// If she has a dick can she really be a lesbian?
	// Negative Traits
	if (girl->has_trait("Broken Will"))	{	girl->m_WorkingDay -= g_Dice.bell(10, 20);	ss << "She just sits there doing exactly what you tell her to do, You don't think it is really getting through to her.\n"; }
	if (girl->has_trait("Mind Fucked"))		girl->m_WorkingDay -= g_Dice.bell(10, 20);	// Does she even know who is fucking her?
	if (girl->has_trait("Retarded"))		girl->m_WorkingDay -= g_Dice.bell(5, 10);	// Does she even know who is fucking her?
	if (girl->has_trait("Slow Learner"))	girl->m_WorkingDay -= g_Dice % 10;			//
	if (girl->has_trait("Iron Will"))		girl->m_WorkingDay -= g_Dice % 5;			// She is set in her ways

	if (girl->has_trait("Broodmother"))		girl->m_WorkingDay -= g_Dice % 20;			// She prefers males who can get her pregnant
	if (girl->has_trait("Porn Star"))		girl->m_WorkingDay -= g_Dice % 10;			// She is used to having sex with anyone her director tells her to
	if (girl->has_trait("Succubus"))		girl->m_WorkingDay -= g_Dice % 10;			// Males are easier to drain energy from
	if (girl->has_trait("Your Wife"))		girl->m_WorkingDay -= g_Dice % 10;			// She wants to be with you (this will changed when player gender is added)
	if (girl->has_trait("Shape Shifter"))	girl->m_WorkingDay -= g_Dice % 10;			// If she can become anyone can she really be a lesbian?
	if (girl->has_trait("Whore"))			girl->m_WorkingDay -= g_Dice % 5;			// She'll do anyone as long as they can pay
	if (girl->has_trait("Slut"))			girl->m_WorkingDay -= g_Dice % 5;			// She'll do anyone 
	if (girl->has_trait("Actress"))			girl->m_WorkingDay -= g_Dice % 5;			// She will do whatever her director tells her to

	//	if (g_Girls.CheckVirginity(girl))				{}

	if (g_Girls.DisobeyCheck(girl, actiontype, brothel))	girl->m_WorkingDay /= 2;	// if she disobeys, half her time is wasted

#pragma endregion
#pragma region //	Count the Days				//

	if (girl->m_WorkingDay <= 0)
	{
		girl->m_WorkingDay = 0;
		msgtype = EVENT_WARNING;
		ss << "\nShe resisted all attempts to make her a Lesbian.";
		wages = 0;
	}
	else if (girl->m_WorkingDay >= 100 && Day0Night1)
	{
		girl->m_PrevWorkingDay = girl->m_WorkingDay = 0;
		msgtype = EVENT_GOODNEWS;
		ss << "\nHer Sexual Orientation conversion is complete. She is now a Lesbian.";
		girl->add_trait("Lesbian");	girl->remove_trait("Bisexual");	girl->remove_trait("Straight");
		girl->m_PrevDayJob = girl->m_PrevNightJob = girl->m_YesterDayJob = girl->m_YesterNightJob = girl->m_DayJob = girl->m_NightJob = JOB_HOUSEREST;
		wages = 200;
	}
	else
	{
		ss << "Her Sexual Orientation conversion to Lesbian is in progress (" << girl->m_WorkingDay << "%).";
		wages = min(100, girl->m_WorkingDay);
	}

#pragma endregion
#pragma region	//	Finish the shift			//

	girl->m_Events.AddMessage(ss.str(), imagetype, msgtype);

	if (girl->is_slave()) wages /= 2;
	girl->m_Pay = wages;

	// Improve girl
	girl->lesbian(g_Dice.bell(2, 15));
	girl->normalsex(g_Dice.bell(-5, -15));
	girl->group(g_Dice.bell(0, -5));
	girl->anal(g_Dice.bell(0, -5));
	girl->oralsex(g_Dice.bell(-1, 5));
	girl->handjob(g_Dice.bell(-3, 3));
	girl->tittysex(g_Dice.bell(-5, 1));

	libido += girl->has_trait("Nymphomaniac") ? 3 : 1;
	g_Girls.UpdateStatTemp(girl, STAT_LIBIDO, libido);
	g_Girls.UpdateEnjoyment(girl, actiontype, enjoy);

#pragma endregion
	return false;
}

double cJobManager::JP_SOLesbian(sGirl* girl, bool estimate)// not used
{
	if (g_Girls.HasTrait(girl, "Lesbian"))	return -1000;
	return 250;
}