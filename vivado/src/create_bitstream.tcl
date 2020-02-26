if {! [file exist zynqmp-rocket]} {
  puts "Recreating Vivado project..."
  source src/create_project.tcl
}
open_project zynqmp-rocket/zynqmp-rocket.xpr
update_compile_order -fileset sources_1
launch_runs impl_1 -to_step write_bitstream -jobs [exec nproc]
wait_on_run impl_1 -quiet