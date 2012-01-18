﻿/*
===========================================================================

  Copyright (c) 2010-2012 Darkstar Dev Teams

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/

  This file is part of DarkStar-server source code.

===========================================================================
*/

#include <string.h>

#include "itemutils.h"
#include "universal_container.h"


/************************************************************************
*																		*
*  Конструктор															*
*																		*
************************************************************************/

CUContainer::CUContainer()
{
	Clean();
}

/************************************************************************
*																		*
*  Отчищаем контейнер													*
*																		*
************************************************************************/

void CUContainer::Clean(bool NeedDelete)
{
	m_ContainerType = UCONTAINER_EMPTY;

    if (NeedDelete)
    {
        for (uint8 i = 0; i < UCONTAINER_SIZE; ++i)
        {
            delete m_PItem[i];
        }
    }
	memset(m_PItem, 0, sizeof(m_PItem));
}

/************************************************************************
*																		*
*  Узнаем текущий тип контейнера										*
*																		*
************************************************************************/

UCONTAINERTYPE CUContainer::GetType()
{
	return m_ContainerType;
}

/************************************************************************
*																		*
*  Устанавливаем текущий тип контейнера									*
*																		*
************************************************************************/

void CUContainer::SetType(UCONTAINERTYPE Type)
{
	DSP_DEBUG_BREAK_IF(m_ContainerType != UCONTAINER_EMPTY);

	m_ContainerType = Type;
}

/************************************************************************
*																		*
*  Проверяем, пуст ли контейнер											*
*																		*
************************************************************************/

bool CUContainer::IsContainerEmpty()
{
	return (m_ContainerType == UCONTAINER_EMPTY);
}

/************************************************************************
*                                                                       *
*  Проверяем, пуста ли ячейка                                           *
*                                                                       *
************************************************************************/

bool CUContainer::IsSlotEmpty(uint8 slotID)
{
    if (slotID < UCONTAINER_SIZE)
	{
        return m_PItem[slotID] == NULL;
    }
    return true;
}

/************************************************************************
*																		*
*  Добавляем предмет в указанныю ячейку контейнера						*
*																		*
************************************************************************/

void CUContainer::SetItem(uint8 slotID, CItem* PItem)
{
	if (slotID < UCONTAINER_SIZE)
	{
		m_PItem[slotID] = PItem;
	}
}

/************************************************************************
*																		*
*  Получаем предмет из указанной ячейки контейнера						*
*																		*
************************************************************************/

CItem* CUContainer::GetItem(uint8 slotID)
{
	if (slotID < UCONTAINER_SIZE)
	{
		return m_PItem[slotID];
	}
	return NULL;
}