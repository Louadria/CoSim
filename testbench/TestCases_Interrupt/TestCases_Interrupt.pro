#  File Name:         testbench.pro
#  Revision:          STANDARD VERSION
#
#  Maintainer:        Simon Southwell email:  simon.southwell@gmail.com
#  Contributor(s):
#     Simon Southwell simon.southwell@gmail.com
#     Jim Lewis       jim@synthworks.com
#
#
#  Description:
#        Script to run one Co-simulation Interrupt test
#
#  Developed for:
#        SynthWorks Design Inc.
#        VHDL Training Classes
#        11898 SW 128th Ave.  Tigard, Or  97223
#        http://www.SynthWorks.com
#
#  Revision History:
#    Date      Version    Description
#    12/2022   2023.01    Updated after refactoring and moving of testbenches to CoSim directory
#    11/2022              Initial
#
#
#  This file is part of OSVVM.
#  
#  Copyright (c) 2022 by SynthWorks Design Inc.  
#  
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#  
#      https://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#  

# library and TestSuite set by higher level scripts

analyze TbAxi4_InterruptCosim1.vhd
MkVproc ${::osvvm::SCRIPT_DIR}/../CoSim  tests/interrupt
simulate TbAxi4_InterruptCosim1

analyze TbAxi4_InterruptCosim2.vhd
MkVproc ${::osvvm::SCRIPT_DIR}/../CoSim  tests/interruptCB
simulate TbAxi4_InterruptCosim2

analyze TbAxi4_InterruptCosim3.vhd
MkVproc ${::osvvm::SCRIPT_DIR}/../CoSim  tests/interruptIss rv32
simulate TbAxi4_InterruptCosim3

