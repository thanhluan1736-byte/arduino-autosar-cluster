"""Writes model/Cluster.arxml for the Arduino AUTOSAR cluster.

Usage: python tools/make_arxml.py model/Cluster.arxml   (then tools/gen_rte.ps1)
"""
import sys

PKG = "/AppPackage"

types = [  # name, base, size, native
    ("dt_uint8", "uint8", 8, "uint8"),
    ("dt_uint16", "uint16", 16, "uint16"),
    ("dt_uint32", "uint32", 32, "unsigned long"),
]

ifaces = {  # name: [(element, type)]
    "IF_Pedal": [("Element_Throttle", "dt_uint8"), ("Element_Brake", "dt_uint8")],
    "IF_ButtonEvent": [("Element_Event", "dt_uint8"), ("Element_Seq", "dt_uint8")],
    "IF_VehicleSpeed": [("Element_Speed", "dt_uint16")],
    "IF_Distance": [("Element_Odo", "dt_uint32"), ("Element_Trip", "dt_uint32"), ("Element_NvStatus", "dt_uint8")],
    "IF_TripReset": [("Element_ReqSeq", "dt_uint8"), ("Element_OdoReqSeq", "dt_uint8")],
}

# swc: ports [(kind P/R, name, iface)], runnables [(name, event, sends[(port, el)], reads[(port, el)])]
# event: ("TE", seconds) or ("DRE", port, element)
swcs = [
    ("SWC_DriverInput",
     [("P", "Pp_Pedal", "IF_Pedal"), ("P", "Pp_ButtonEvent", "IF_ButtonEvent")],
     [("Runnable_DriverInput_10ms", ("TE", "0.01"),
       [("Pp_Pedal", "Element_Throttle"), ("Pp_Pedal", "Element_Brake"), ("Pp_ButtonEvent", "Element_Event"),
        ("Pp_ButtonEvent", "Element_Seq")], [])]),
    ("SWC_VehicleModel",
     [("R", "Rp_Pedal", "IF_Pedal"), ("P", "Pp_SpeedGauge", "IF_VehicleSpeed"), ("P", "Pp_SpeedOdo", "IF_VehicleSpeed"),
      ("P", "Pp_SpeedDisplay", "IF_VehicleSpeed")],
     [("Runnable_VehicleModel_10ms", ("TE", "0.01"),
       [("Pp_SpeedGauge", "Element_Speed"), ("Pp_SpeedOdo", "Element_Speed"), ("Pp_SpeedDisplay", "Element_Speed")],
       [("Rp_Pedal", "Element_Throttle"), ("Rp_Pedal", "Element_Brake")])]),
    ("SWC_Gauge",
     [("R", "Rp_SpeedGauge", "IF_VehicleSpeed")],
     [("Runnable_Gauge_20ms", ("TE", "0.02"), [], [("Rp_SpeedGauge", "Element_Speed")])]),
    ("SWC_Odometer",
     [("R", "Rp_SpeedOdo", "IF_VehicleSpeed"), ("P", "Pp_Distance", "IF_Distance"), ("R", "Rp_TripReset", "IF_TripReset")],
     [("Runnable_Odometer_100ms", ("TE", "0.1"),
       [("Pp_Distance", "Element_Odo"), ("Pp_Distance", "Element_Trip"), ("Pp_Distance", "Element_NvStatus")],
       [("Rp_SpeedOdo", "Element_Speed"), ("Rp_TripReset", "Element_ReqSeq"), ("Rp_TripReset", "Element_OdoReqSeq")])]),
    ("SWC_Display",
     [("R", "Rp_Distance", "IF_Distance"), ("R", "Rp_ButtonEvent", "IF_ButtonEvent"), ("P", "Pp_TripReset", "IF_TripReset"),
      ("R", "Rp_SpeedDisplay", "IF_VehicleSpeed")],
     [("Runnable_Display_100ms", ("TE", "0.1"), [("Pp_TripReset", "Element_ReqSeq"), ("Pp_TripReset", "Element_OdoReqSeq")],
       [("Rp_Distance", "Element_Odo"), ("Rp_Distance", "Element_Trip"), ("Rp_Distance", "Element_NvStatus"),
        ("Rp_ButtonEvent", "Element_Event"), ("Rp_ButtonEvent", "Element_Seq"), ("Rp_SpeedDisplay", "Element_Speed")])]),
]

# connectors: (name, provider swc, p-port, requester swc, r-port)
conns = [
    ("Conn_Pedal", "SWC_DriverInput", "Pp_Pedal", "SWC_VehicleModel", "Rp_Pedal"),
    ("Conn_ButtonEvent", "SWC_DriverInput", "Pp_ButtonEvent", "SWC_Display", "Rp_ButtonEvent"),
    ("Conn_SpeedGauge", "SWC_VehicleModel", "Pp_SpeedGauge", "SWC_Gauge", "Rp_SpeedGauge"),
    ("Conn_SpeedOdo", "SWC_VehicleModel", "Pp_SpeedOdo", "SWC_Odometer", "Rp_SpeedOdo"),
    ("Conn_SpeedDisplay", "SWC_VehicleModel", "Pp_SpeedDisplay", "SWC_Display", "Rp_SpeedDisplay"),
    ("Conn_Distance", "SWC_Odometer", "Pp_Distance", "SWC_Display", "Rp_Distance"),
    ("Conn_TripReset", "SWC_Display", "Pp_TripReset", "SWC_Odometer", "Rp_TripReset"),
]

o = []
w = o.append
w('<?xml version="1.0" encoding="UTF-8"?>')
w('<!--\n  Mini AUTOSAR instrument cluster (Arduino Uno / ATmega328P)\n'
  '  SWC_DriverInput -> SWC_VehicleModel -> SWC_Gauge (needle) / SWC_Odometer (NvM) -> SWC_Display (LCD1602)\n'
  '  RTE generated with arxml2rte (rte_generator.py)\n-->')
w('<AUTOSAR xmlns="http://autosar.org/schema/r4.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" '
  'xsi:schemaLocation="http://autosar.org/schema/r4.0 AUTOSAR_4-2-2.xsd">')
w('  <AR-PACKAGES>\n    <AR-PACKAGE>\n      <SHORT-NAME>AppPackage</SHORT-NAME>\n      <ELEMENTS>')
w('        <!-- ===== Data types ===== -->')
for (dt, base, size, native) in types:
    w('        <SW-BASE-TYPE>\n          <SHORT-NAME>%s</SHORT-NAME>\n          <CATEGORY>FIXED_LENGTH</CATEGORY>\n'
      '          <BASE-TYPE-SIZE>%d</BASE-TYPE-SIZE>\n          <BASE-TYPE-ENCODING>NONE</BASE-TYPE-ENCODING>\n'
      '          <NATIVE-DECLARATION>%s</NATIVE-DECLARATION>\n        </SW-BASE-TYPE>' % (base, size, native))
for (dt, base, size, native) in types:
    w('        <IMPLEMENTATION-DATA-TYPE>\n          <SHORT-NAME>%s</SHORT-NAME>\n          <CATEGORY>VALUE</CATEGORY>\n'
      '          <SW-DATA-DEF-PROPS>\n            <SW-DATA-DEF-PROPS-VARIANTS>\n              <SW-DATA-DEF-PROPS-CONDITIONAL>\n'
      '                <BASE-TYPE-REF DEST="SW-BASE-TYPE">%s/%s</BASE-TYPE-REF>\n'
      '              </SW-DATA-DEF-PROPS-CONDITIONAL>\n            </SW-DATA-DEF-PROPS-VARIANTS>\n'
      '          </SW-DATA-DEF-PROPS>\n        </IMPLEMENTATION-DATA-TYPE>' % (dt, PKG, base))
w('\n        <!-- ===== Sender-Receiver interfaces ===== -->')
for name, els in ifaces.items():
    w('        <SENDER-RECEIVER-INTERFACE>\n          <SHORT-NAME>%s</SHORT-NAME>\n          <IS-SERVICE>false</IS-SERVICE>\n'
      '          <DATA-ELEMENTS>' % name)
    for (el, dt) in els:
        w('            <VARIABLE-DATA-PROTOTYPE>\n              <SHORT-NAME>%s</SHORT-NAME>\n'
          '              <TYPE-TREF DEST="IMPLEMENTATION-DATA-TYPE">%s/%s</TYPE-TREF>\n'
          '            </VARIABLE-DATA-PROTOTYPE>' % (el, PKG, dt))
    w('          </DATA-ELEMENTS>\n        </SENDER-RECEIVER-INTERFACE>')


def access(kind, swc, port_kind, port, iface, el, short):
    return ('                      <VARIABLE-ACCESS>\n                        <SHORT-NAME>%s</SHORT-NAME>\n'
            '                        <ACCESSED-VARIABLE>\n                          <AUTOSAR-VARIABLE-IREF>\n'
            '                            <PORT-PROTOTYPE-REF DEST="%s-PORT-PROTOTYPE">%s/%s/%s</PORT-PROTOTYPE-REF>\n'
            '                            <TARGET-DATA-PROTOTYPE-REF DEST="VARIABLE-DATA-PROTOTYPE">%s/%s/%s</TARGET-DATA-PROTOTYPE-REF>\n'
            '                          </AUTOSAR-VARIABLE-IREF>\n                        </ACCESSED-VARIABLE>\n'
            '                      </VARIABLE-ACCESS>' % (short, port_kind, PKG, swc, port, PKG, iface, el))


for (swc, ports, runs) in swcs:
    pif = {p[1]: p[2] for p in ports}
    w('\n        <!-- ===== %s ===== -->' % swc)
    w('        <APPLICATION-SW-COMPONENT-TYPE>\n          <SHORT-NAME>%s</SHORT-NAME>\n          <PORTS>' % swc)
    for (k, pn, ifn) in ports:
        if k == "P":
            w('            <P-PORT-PROTOTYPE>\n              <SHORT-NAME>%s</SHORT-NAME>\n'
              '              <PROVIDED-INTERFACE-TREF DEST="SENDER-RECEIVER-INTERFACE">%s/%s</PROVIDED-INTERFACE-TREF>\n'
              '            </P-PORT-PROTOTYPE>' % (pn, PKG, ifn))
        else:
            w('            <R-PORT-PROTOTYPE>\n              <SHORT-NAME>%s</SHORT-NAME>\n'
              '              <REQUIRED-INTERFACE-TREF DEST="SENDER-RECEIVER-INTERFACE">%s/%s</REQUIRED-INTERFACE-TREF>\n'
              '            </R-PORT-PROTOTYPE>' % (pn, PKG, ifn))
    ib = "IB_" + swc
    w('          </PORTS>\n          <INTERNAL-BEHAVIORS>\n            <SWC-INTERNAL-BEHAVIOR>\n'
      '              <SHORT-NAME>%s</SHORT-NAME>\n              <EVENTS>' % ib)
    for (rn, ev, sends, reads) in runs:
        ref = '%s/%s/%s/%s' % (PKG, swc, ib, rn)
        if ev[0] == "TE":
            ms = int(round(float(ev[1]) * 1000))
            w('                <TIMING-EVENT>\n                  <SHORT-NAME>TE_%s_%dms</SHORT-NAME>\n'
              '                  <START-ON-EVENT-REF DEST="RUNNABLE-ENTITY">%s</START-ON-EVENT-REF>\n'
              '                  <PERIOD>%s</PERIOD>\n                </TIMING-EVENT>' % (swc[4:], ms, ref, ev[1]))
        else:
            w('                <DATA-RECEIVED-EVENT>\n                  <SHORT-NAME>DRE_%s</SHORT-NAME>\n'
              '                  <START-ON-EVENT-REF DEST="RUNNABLE-ENTITY">%s</START-ON-EVENT-REF>\n'
              '                  <DATA-IREF>\n'
              '                    <CONTEXT-R-PORT-REF DEST="R-PORT-PROTOTYPE">%s/%s/%s</CONTEXT-R-PORT-REF>\n'
              '                    <TARGET-DATA-ELEMENT-REF DEST="VARIABLE-DATA-PROTOTYPE">%s/%s/%s</TARGET-DATA-ELEMENT-REF>\n'
              '                  </DATA-IREF>\n                </DATA-RECEIVED-EVENT>'
              % (ev[1][3:], ref, PKG, swc, ev[1], PKG, pif[ev[1]], ev[2]))
    w('              </EVENTS>\n              <RUNNABLES>')
    for (rn, ev, sends, reads) in runs:
        w('                <RUNNABLE-ENTITY>\n                  <SHORT-NAME>%s</SHORT-NAME>\n'
          '                  <MINIMUM-START-INTERVAL>0</MINIMUM-START-INTERVAL>\n'
          '                  <CAN-BE-INVOKED-CONCURRENTLY>false</CAN-BE-INVOKED-CONCURRENTLY>' % rn)
        if reads:
            w('                  <DATA-RECEIVE-POINT-BY-ARGUMENTS>')
            for (pn, el) in reads:
                w(access("R", swc, "R", pn, pif[pn], el, "DRP_%s_%s" % (pn[3:], el[8:])))
            w('                  </DATA-RECEIVE-POINT-BY-ARGUMENTS>')
        if sends:
            w('                  <DATA-SEND-POINTS>')
            for (pn, el) in sends:
                w(access("P", swc, "P", pn, pif[pn], el, "DSP_%s_%s" % (pn[3:], el[8:])))
            w('                  </DATA-SEND-POINTS>')
        w('                  <SYMBOL>%s</SYMBOL>\n                </RUNNABLE-ENTITY>' % rn)
    w('              </RUNNABLES>\n            </SWC-INTERNAL-BEHAVIOR>\n          </INTERNAL-BEHAVIORS>\n'
      '        </APPLICATION-SW-COMPONENT-TYPE>')

comp = "Cluster_Composition"
w('\n        <!-- ===== Composition + VFB connectors ===== -->')
w('        <COMPOSITION-SW-COMPONENT-TYPE>\n          <SHORT-NAME>%s</SHORT-NAME>\n          <COMPONENTS>' % comp)
for (swc, _, _) in swcs:
    w('            <SW-COMPONENT-PROTOTYPE>\n              <SHORT-NAME>CP_%s</SHORT-NAME>\n'
      '              <TYPE-TREF DEST="APPLICATION-SW-COMPONENT-TYPE">%s/%s</TYPE-TREF>\n'
      '            </SW-COMPONENT-PROTOTYPE>' % (swc[4:], PKG, swc))
w('          </COMPONENTS>\n          <CONNECTORS>')
for (cn, ps, pp, rs, rp) in conns:
    w('            <ASSEMBLY-SW-CONNECTOR>\n              <SHORT-NAME>%s</SHORT-NAME>\n              <PROVIDER-IREF>\n'
      '                <CONTEXT-COMPONENT-REF DEST="SW-COMPONENT-PROTOTYPE">%s/%s/CP_%s</CONTEXT-COMPONENT-REF>\n'
      '                <TARGET-P-PORT-REF DEST="P-PORT-PROTOTYPE">%s/%s/%s</TARGET-P-PORT-REF>\n'
      '              </PROVIDER-IREF>\n              <REQUESTER-IREF>\n'
      '                <CONTEXT-COMPONENT-REF DEST="SW-COMPONENT-PROTOTYPE">%s/%s/CP_%s</CONTEXT-COMPONENT-REF>\n'
      '                <TARGET-R-PORT-REF DEST="R-PORT-PROTOTYPE">%s/%s/%s</TARGET-R-PORT-REF>\n'
      '              </REQUESTER-IREF>\n            </ASSEMBLY-SW-CONNECTOR>'
      % (cn, PKG, comp, ps[4:], PKG, ps, pp, PKG, comp, rs[4:], PKG, rs, rp))
w('          </CONNECTORS>\n        </COMPOSITION-SW-COMPONENT-TYPE>')
w('      </ELEMENTS>\n    </AR-PACKAGE>\n  </AR-PACKAGES>\n</AUTOSAR>\n')

with open(sys.argv[1], "w", encoding="utf-8", newline="\n") as f:
    f.write("\n".join(o))
print("written", sys.argv[1])
