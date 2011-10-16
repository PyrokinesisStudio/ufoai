/**
 * @file e_time.c
 * @brief Battlescape event timing code
 */

/*
Copyright (C) 2002-2011 UFO: Alien Invasion.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "../../client.h"
#include "../cl_localentity.h"
#include "e_main.h"
#include "e_time.h"

/** @todo remove the old event timing */
#define OLDEVENTTIME

/**
 * @brief Calculates the time the event should get executed. If two events return the same time,
 * they are going to be executed in the order the were parsed.
 * @param[in] eType The event type
 * @param[in,out] msg The message buffer that can be modified to get the event time
 * @param[in,out] eventTiming The delay data for the events
 * @return the time the event should be executed. This value is used to sort the
 * event chain to determine which event must be executed at first. This
 * value also ensures, that the events are executed in the correct
 * order. E.g. @c impactTime is used to delay some events in case the
 * projectile needs some time to reach its target.
 */
int CL_GetEventTime (const event_t eType, struct dbuffer *msg, eventTiming_t *eventTiming)
{
	const eventRegister_t *eventData = CL_GetEvent(eType);
	int eventTime;

	/* get event time */
	if (eventTiming->nextTime < cl.time)
		eventTiming->nextTime = cl.time;
	if (eventTiming->impactTime < cl.time)
		eventTiming->impactTime = cl.time;

#ifdef OLDEVENTTIME
	if (eType == EV_ACTOR_DIE)
		eventTiming->parsedDeath = qtrue;

	if (eType == EV_ACTOR_DIE || eType == EV_MODEL_EXPLODE)
		eventTime = eventTiming->impactTime;
	else if (eType == EV_ACTOR_SHOOT || eType == EV_ACTOR_SHOOT_HIDDEN)
		eventTime = eventTiming->shootTime;
	else if (eType == EV_RESULTS)
		eventTime = eventTiming->nextTime + 1400;
	else
		eventTime = eventTiming->nextTime;

	if (eType == EV_ENT_APPEAR || eType == EV_INV_ADD || eType == EV_PARTICLE_APPEAR || eType == EV_PARTICLE_SPAWN) {
		if (eventTiming->parsedDeath) { /* drop items after death (caused by impact) */
			eventTime = eventTiming->impactTime + 400;
			/* EV_INV_ADD messages are the last events sent after a death */
			if (eType == EV_INV_ADD)
				eventTiming->parsedDeath = qfalse;
		} else if (eventTiming->impactTime > cl.time) { /* item thrown on the ground */
			eventTime = eventTiming->impactTime + 75;
		}
	}

	/* calculate time interval before the next event */
	switch (eType) {
	case EV_ACTOR_APPEAR:
		if (cl.actTeam != cls.team)
			eventTiming->nextTime += 600;
		break;
	case EV_INV_RELOAD:
		/* let the reload sound play */
		eventTiming->nextTime += 600;
		break;
	case EV_ACTOR_START_SHOOT:
		eventTiming->nextTime += 300;
		eventTiming->shootTime = eventTiming->nextTime;
		break;
	case EV_ACTOR_SHOOT_HIDDEN:
		{
			int first;
			int objIdx;
			const objDef_t *obj;
			weaponFireDefIndex_t weapFdsIdx;
			fireDefIndex_t fireDefIndex;

			NET_ReadFormat(msg, eventData->formatString, &first, &objIdx, &weapFdsIdx, &fireDefIndex);

			obj = INVSH_GetItemByIDX(objIdx);
			if (first) {
				eventTiming->nextTime += 500;
				eventTiming->impactTime = eventTiming->shootTime = eventTiming->nextTime;
			} else {
				const fireDef_t *fd = FIRESH_GetFiredef(obj, weapFdsIdx, fireDefIndex);
				/* impact right away - we don't see it at all
				 * bouncing is not needed here, too (we still don't see it) */
				eventTiming->impactTime = eventTiming->shootTime;
				eventTiming->nextTime = eventTiming->shootTime + 1400;
				if (fd->delayBetweenShots > 0.0)
					eventTiming->shootTime += 1000 / fd->delayBetweenShots;
			}
			eventTiming->parsedDeath = qfalse;
		}
		break;
	case EV_ACTOR_MOVE:
		{
			le_t *le;
			int number, i;
			int time = 0;
			int pathLength;
			byte crouchingState;
			pos3_t pos, oldPos;

			number = NET_ReadShort(msg);
			/* get le */
			le = LE_Get(number);
			if (!le)
				LE_NotFoundError(number);

			pathLength = NET_ReadByte(msg);

			/* Also skip the final position */
			NET_ReadByte(msg);
			NET_ReadByte(msg);
			NET_ReadByte(msg);

			VectorCopy(le->pos, pos);
			crouchingState = LE_IsCrouched(le) ? 1 : 0;

			for (i = 0; i < pathLength; i++) {
				const dvec_t dvec = NET_ReadShort(msg);
				const byte dir = getDVdir(dvec);
				VectorCopy(pos, oldPos);
				PosAddDV(pos, crouchingState, dvec);
				time += LE_ActorGetStepTime(le, pos, oldPos, dir, NET_ReadShort(msg));
				NET_ReadShort(msg);
			}
			eventTiming->nextTime += time + 400;
		}
		break;
	case EV_ACTOR_SHOOT:
		{
			const fireDef_t	*fd;
			int flags, dummy;
			int objIdx, surfaceFlags;
			const objDef_t *obj;
			int weap_fds_idx, fd_idx;
			shoot_types_t shootType;
			vec3_t muzzle, impact;

			/* read data */
			NET_ReadFormat(msg, eventData->formatString, &dummy, &dummy, &dummy, &objIdx, &weap_fds_idx, &fd_idx, &shootType, &flags, &surfaceFlags, &muzzle, &impact, &dummy);

			obj = INVSH_GetItemByIDX(objIdx);
			fd = FIRESH_GetFiredef(obj, weap_fds_idx, fd_idx);

			if (!(flags & SF_BOUNCED)) {
				/* shooting */
				if (fd->speed > 0.0 && !CL_OutsideMap(impact, UNIT_SIZE * 10)) {
					eventTiming->impactTime = eventTiming->shootTime + 1000 * VectorDist(muzzle, impact) / fd->speed;
				} else {
					eventTiming->impactTime = eventTiming->shootTime;
				}
				if (cl.actTeam != cls.team)
					eventTiming->nextTime = eventTiming->impactTime + 1400;
				else
					eventTiming->nextTime = eventTiming->impactTime + 400;
				if (fd->delayBetweenShots > 0.0)
					eventTiming->shootTime += 1000 / fd->delayBetweenShots;
			} else {
				/* only a bounced shot */
				eventTime = eventTiming->impactTime;
				if (fd->speed > 0.0) {
					eventTiming->impactTime += 1000 * VectorDist(muzzle, impact) / fd->speed;
					eventTiming->nextTime = eventTiming->impactTime;
				}
			}
			eventTiming->parsedDeath = qfalse;
		}
		break;
	case EV_ACTOR_THROW:
		eventTiming->nextTime += NET_ReadShort(msg);
		eventTiming->impactTime = eventTiming->shootTime = eventTiming->nextTime;
		eventTiming->parsedDeath = qfalse;
		break;
	default:
		break;
	}
#else
	if (!eventData->timeCallback)
		eventTime = eventTiming->nextTime;
	else
		eventTime = eventData->timeCallback(eventData, msg, eventTiming);
#endif

	Com_DPrintf(DEBUG_EVENTSYS, "%s => eventTime: %i, nextTime: %i, impactTime: %i, shootTime: %i, cl.time: %i\n",
			eventData->name, eventTime, eventTiming->nextTime, eventTiming->impactTime, eventTiming->shootTime, cl.time);

	return eventTime;
}
