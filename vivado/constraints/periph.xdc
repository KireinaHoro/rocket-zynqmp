# PL UART
set_property IOSTANDARD LVCMOS33 [get_ports uart_pl_*]
# pin 10
set_property PACKAGE_PIN A14 [get_ports uart_pl_rxd]
# pin 9
set_property PACKAGE_PIN B14 [get_ports uart_pl_txd]
# PL LED
set_property IOSTANDARD LVCMOS33 [get_ports {led_pl[0]}]
set_property PACKAGE_PIN A10 [get_ports {led_pl[0]}]

# PL Refclock
set_property IOSTANDARD LVCMOS18 [get_ports refclk_pl]
set_property PACKAGE_PIN D4 [get_ports refclk_pl]
