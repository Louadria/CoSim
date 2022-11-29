--
--  File Name:         OssvmTestCoSimPkg.vhd
--  Design Unit Name:  OssvmTestCoSimPkg
--  Revision:          OSVVM MODELS STANDARD VERSION
--
--  Maintainer:        Simon Southwell  email: simon.southwell@gmail.com
--  Contributor(s):
--     Jim Lewis            jim@synthworks.com
--     Simon Southwell      simon.southwell@gmail.com
--
--
--  Description:
--      Defines procedures to support co-simulation
--
--
--  Developed by:
--        SynthWorks Design Inc.
--        VHDL Training Classes
--        http://www.SynthWorks.com
--
--  Revision History:
--    Date      Version    Description
--    09/2022   2022       Initial revision
--
--
--  This file is part of OSVVM.
--
--  Copyright (c) 2022 by SynthWorks Design Inc.
--
--  Licensed under the Apache License, Version 2.0 (the "License");
--  you may not use this file except in compliance with the License.
--  You may obtain a copy of the License at
--
--      https://www.apache.org/licenses/LICENSE-2.0
--
--  Unless required by applicable law or agreed to in writing, software
--  distributed under the License is distributed on an "AS IS" BASIS,
--  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
--  See the License for the specific language governing permissions and
--  limitations under the License.
--

library ieee ;
  use ieee.std_logic_1164.all ;
  use ieee.numeric_std.all ;
  use ieee.numeric_std_unsigned.all ;

library OSVVM ;
  context OSVVM.OsvvmContext ;
  use osvvm.ScoreboardPkg_slv.all ;

library OSVVM_Common ;
  use OSVVM_Common.AddressBusTransactionPkg.all ;

library osvvm_tbcosim ;
  use osvvm_TbCosim.OsvvmVprocPkg.all ;

package OsvvmTestCoSimPkg is

constant WEbit           : integer := 0 ;
constant RDbit           : integer := 1 ;
constant NodeNum         : integer := 0  ; -- Always use node 0 for now
constant ADDR_WIDTH      : integer := 32 ;
constant DATA_WIDTH      : integer := 32 ;
constant ADDR_WIDTH_MAX  : integer := 64 ;
constant DATA_WIDTH_MAX  : integer := 64 ;

  ------------------------------------------------------------
  ------------------------------------------------------------

procedure CoSimTrans (
  signal   ManagerRec       : inout  AddressBusRecType ;
  variable Ticks            : inout  integer ;
  variable Done             : inout  integer ;
  variable Error            : inout  integer
  ) ;

procedure CoSimTransSingle (
  signal   ManagerRec       : inout  AddressBusRecType ;
  variable VPDataOut        : in     integer ;
  variable VPDataOutHi      : in     integer ;
  variable VPDataWidth      : in     integer ;
  variable VPAddr           : in     integer ;
  variable VPAddrHi         : in     integer ;
  variable VPAddrWidth      : in     integer ;
  variable VPRW             : in     integer
  ) ;

procedure CoSimTransBurst (
  signal   ManagerRec       : inout  AddressBusRecType ;
  variable VPDataWidth      : in     integer ;
  variable VPAddr           : in     integer ;
  variable VPAddrHi         : in     integer ;
  variable VPAddrWidth      : in     integer ;
  variable VPRW             : in     integer ;
  variable VPBurstSize      : in     integer
  ) ;

function squelchUndef (
           vec              : in     std_logic_vector
  ) return std_logic_vector ;

end package OsvvmTestCoSimPkg ;

-- /////////////////////////////////////////////////////////////////////////////////////////
-- /////////////////////////////////////////////////////////////////////////////////////////

package body OsvvmTestCoSimPkg is

  ------------------------------------------------------------
  -- Utility function to convert a std_logic_vector's bits
  -- from 'U' to '0' prior to conversion to integer to
  -- avoid warnings in NUMERIC_STD.TO_INTEGER
  ------------------------------------------------------------
  function squelchUndef (
           vec              : in  std_logic_vector 
  ) return std_logic_vector is

  variable result           : std_logic_vector(vec'length-1 downto 0) ;

  begin
    for i in 0 to vec'length-1 loop
      if vec(i) = 'U' then
        result(i) := '0' ;
      else
        result(i) := vec(i) ;
      end if ;
    end loop;

    return result ;

  end function squelchUndef ;

  ------------------------------------------------------------
  -- Co-simulation wrapper procedure to send read and write
  -- transactions
  --
  -- Note: The ticks parameter is to allow the internally set
  -- state to persist between calls and must be connected to
  -- an integer variable in the process where CoSimTrans is
  -- called. It should not be changed in the external process.
  -- It is used to allow time to advance without instigating
  -- a transaction.
  --
  ------------------------------------------------------------

procedure CoSimTrans (
  -- Transaction  interface
  signal   ManagerRec      : inout  AddressBusRecType ;
  variable Ticks           : inout  integer ;
  variable Done            : inout  integer ;
  variable Error           : inout  integer
  ) is

  variable RdData          : std_logic_vector (DATA_WIDTH_MAX-1 downto 0) ;

  variable VPDataIn        : integer ;
  variable VPDataInHi      : integer ;
  variable VPDataOut       : integer ;
  variable VPDataOutHi     : integer ;
  variable VPDataWidth     : integer ;
  variable VPAddr          : integer ;
  variable VPAddrHi        : integer ;
  variable VPAddrWidth     : integer ;
  variable VPRW            : integer ;
  variable VPBurstSize     : integer ;
  variable VPTicks         : integer ;
  variable VPDone          : integer ;
  variable VPError         : integer ;

  variable Interrupt       : integer := 0 ; -- currently unused

  begin

    -- RdData won't have persisted from last call, so re-fetch from ManagerRec
    -- which will have persisted (and is not yet updated)
    RdData       := squelchUndef(SafeResize(ManagerRec.DataFromModel, RdData'length)) ;

    if Ticks <= 0 then

      -- Sample the read data from last access, saved in RdData inout port
      if ManagerRec.DataWidth > 32 then
        VPDataIn   := to_integer(signed(RdData(31 downto  0))) ;
        VPDataInHi := to_integer(signed(RdData(RdData'length-1 downto 32))) ;
      else
        VPDataIn   := to_integer(signed(RdData(31 downto 0))) ;
        VPDataInHi := 0 ;
      end if;

      -- Call VTrans to generate a new access
      VTrans(NodeNum,   Interrupt,
             VPDataIn,  VPDataInHi,
             VPDataOut, VPDataOutHi, VPDataWidth,
             VPAddr,    VPAddrHi,    VPAddrWidth,
             VPRW,      VPBurstSize, VPTicks,
             VPDone,    VPError) ;

      Ticks := VPTicks ;
      Done  := VPDone  when Done  = 0;  -- Sticky
      Error := VPError when Error = 0;  -- Sticky

      if VPBurstSize = 0 then

        CoSimTransSingle(ManagerRec,
                         VPDataOut, VPDataOutHi, VPDataWidth,
                         VPAddr,    VPAddrHi,    VPAddrWidth,
                         VPRW) ;
      else
        CoSimTransBurst (ManagerRec,
                         VPDataWidth,
                         VPAddr,    VPAddrHi,    VPAddrWidth,
                         VPRW,      VPBurstSize) ;
      end if;

    else

      Ticks := Ticks - 1 ;

    end if ;

  end procedure CoSimTrans ;

  ------------------------------------------------------------
  -- Co-simulation wrapper procedure to send read and write
  -- single word or sub-word transfers
  ------------------------------------------------------------
procedure CoSimTransSingle (

  -- Transaction  interface
  signal   ManagerRec      : inout  AddressBusRecType ;
  variable VPDataOut       : in     integer ;
  variable VPDataOutHi     : in     integer ;
  variable VPDataWidth     : in     integer ;
  variable VPAddr          : in     integer ;
  variable VPAddrHi        : in     integer ;
  variable VPAddrWidth     : in     integer ;
  variable VPRW            : in     integer
  ) is

  variable RdData          : std_logic_vector (DATA_WIDTH_MAX-1 downto 0) ;
  variable WrData          : std_logic_vector (DATA_WIDTH_MAX-1 downto 0) ;
  variable Address         : std_logic_vector (ADDR_WIDTH_MAX-1 downto 0) ;

  begin

    -- Convert address and write data to std_logic_vectors
    Address(31 downto  0) := std_logic_vector(to_signed(VPAddr,      32)) ;
    Address(63 downto 32) := std_logic_vector(to_signed(VPAddrHi,    32)) ;

    WrData(31 downto 0 )  := std_logic_vector(to_signed(VPDataOut,   32)) ;
    WrData(63 downto 32)  := std_logic_vector(to_signed(VPDataOutHi, 32)) ;

    -- Do the operation using the transaction interface
    if VPRW /= 0 then
      if to_unsigned(VPRW, 2)(RDbit)
      then
        if VPAddrWidth = 64 then
          case VPDataWidth is
          when 64 => Read  (ManagerRec, Address, RdData) ;
          when 32 => Read  (ManagerRec, Address, RdData(31 downto 0)) ;
          when 16 => Read  (ManagerRec, Address, RdData(15 downto 0)) ;
          when  8 => Read  (ManagerRec, Address, RdData( 7 downto 0)) ;
          when others => AlertIf(ALERTLOG_DEFAULT_ID, true, "Invalid data width for co-sim read transaction (64 bit arch)");
          end case ;
        elsif VPAddrWidth = 32 then
          case VPDataWidth is
          when 32 => Read  (ManagerRec, Address(31 downto 0), RdData(31 downto 0)) ;
          when 16 => Read  (ManagerRec, Address(31 downto 0), RdData(15 downto 0)) ;
          when  8 => Read  (ManagerRec, Address(31 downto 0), RdData(7 downto 0)) ;
          when others => AlertIf(ALERTLOG_DEFAULT_ID, true, "Invalid data width for co-sim read transaction (32 bit arch)");
          end case ;
        else
          AlertIf(ALERTLOG_DEFAULT_ID, true, "Invalid address width for co-sim read transaction");
        end if ;
      else
        if VPAddrWidth = 64 then
          case VPDataWidth is
          when 64 => Write (ManagerRec, Address, WrData) ;
          when 32 => Write (ManagerRec, Address, WrData(31 downto 0)) ;
          when 16 => Write (ManagerRec, Address, WrData(15 downto 0)) ;
          when  8 => Write (ManagerRec, Address, WrData(7 downto 0)) ;
          when others => AlertIf(ALERTLOG_DEFAULT_ID, true, "Invalid data width for co-sim write transaction (64 bit arch)");
          end case ;
        elsif VPAddrWidth = 32 then
          case VPDataWidth is
          when 32 => Write (ManagerRec, Address(31 downto 0), WrData(31 downto 0)) ;
          when 16 => Write (ManagerRec, Address(31 downto 0), WrData(15 downto 0)) ;
          when  8 => Write (ManagerRec, Address(31 downto 0), WrData(7 downto 0)) ;
          when others => AlertIf(ALERTLOG_DEFAULT_ID, true, "Invalid data width for co-sim write transaction (32 bit arch)");
          end case ;
        else
          AlertIf(ALERTLOG_DEFAULT_ID, true, "Invalid address width for co-sim write transaction");
        end if ;
      end if ;
    end if ;

  end procedure CoSimTransSingle ;

  ------------------------------------------------------------
  -- Co-simulation wrapper procedure to send read and write
  -- burst transfers
  ------------------------------------------------------------

procedure CoSimTransBurst (
  signal   ManagerRec       : inout  AddressBusRecType ;
  variable VPDataWidth      : in     integer ;
  variable VPAddr           : in     integer ;
  variable VPAddrHi         : in     integer ;
  variable VPAddrWidth      : in     integer ;
  variable VPRW             : in     integer ;
  variable VPBurstSize      : in     integer
  ) is

  variable Address          : std_logic_vector (ADDR_WIDTH_MAX-1 downto 0) ;
  variable RdData           : std_logic_vector (DATA_WIDTH_MAX-1 downto 0) ;
  variable WrDataInt        : integer ;
  variable RdDataInt        : integer ;
  variable WrDataSigned     : signed (DATA_WIDTH_MAX-1 downto 0);

  begin

    -- Convert address and write data to std_logic_vectors
    Address(31 downto  0) := std_logic_vector(to_signed(VPAddr,      32)) ;
    Address(63 downto 32) := std_logic_vector(to_signed(VPAddrHi,    32)) ;

    -- If a read or write, do the operation using the transaction burst interface
    if VPRW /= 0 then

      if to_unsigned(VPRW, 2)(RDbit)
      then

        -- Initiate the read burst transfer of the appropriate address size
        if VPAddrWidth = 64 then
          ReadBurst(ManagerRec, Address(63 downto  0), VPBurstSize) ;
        elsif VPAddrWidth = 32 then
          ReadBurst(ManagerRec, Address(31 downto  0), VPBurstSize) ;
        else
          AlertIf(ALERTLOG_DEFAULT_ID, true, "Invalid address width for co-sim burst read transaction");
        end if ;

        -- Pop the bytes from the read fifo and write them the the co-sim receive buffer
        for bidx in 0 to VPBurstSize-1 loop
          RdData := (others => '0');
          Pop(ManagerRec.ReadBurstFifo, RdData(7 downto 0)) ;
          RdDataInt := to_integer(unsigned(RdData(7 downto 0))) ;
          VSetBurstRdByte(NodeNum, bidx, RdDataInt) ;
        end loop;

      else

        -- Fetch the bytes from the co-sim send buffer and push to the transaction write fifo
        for bidx in 0 to VPBurstSize-1 loop
          VGetBurstWrByte(NodeNum, bidx, WrDataInt) ;
          WrDataSigned := to_signed(WrDataInt, WrDataSigned'length);
          Push(ManagerRec.WriteBurstFifo, std_logic_vector(WrDataSigned(7 downto 0))) ;
        end loop ;

        -- Initiate the write burst transfer of the appropriate address size
        if VPAddrWidth = 64 then
          WriteBurst(ManagerRec, Address(63 downto  0), VPBurstSize) ;
        elsif VPAddrWidth = 32 then
          WriteBurst(ManagerRec, Address(31 downto  0), VPBurstSize) ;
        else
          AlertIf(ALERTLOG_DEFAULT_ID, true, "Invalid address width for co-sim burst write transaction");
        end if ;

      end if ;

    end if ;

  end procedure CoSimTransBurst;

end package body OsvvmTestCoSimPkg ;