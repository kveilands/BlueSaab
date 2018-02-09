/*
 * Virtual C++ Class for RovingNetworks RN-52 Bluetooth modules
 * Copyright (C) 2013  Tim Otto
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Created by: Tim Otto
 * Created on: Jun 21, 2013
 * Modified by: Sam Thompson
 * Last modified on: Dec 15, 2016
 */

#ifndef RN52CONFIGURATION_H
#define RN52CONFIGURATION_H


#define SPP_TX_BUFFER_SIZE		128
#define CMD_RX_BUFFER_SIZE		68
#define CMD_QUEUE_SIZE			12 // Leave enough room to queue the config cmds in initialize()
#define CMD_TIMEOUT             1000
#define REBOOT_TIMEOUT          3000

#endif /* RN52CONFIGURATION_H */
