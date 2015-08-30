@pushd src\common
@echo in src/common
sgsvm -p ../../ext/src/sgscript/cppbc.sgs scritem.hpp
@popd
@pushd src\tacstrike
@echo in src/tacstrike
sgsvm -p ../../ext/src/sgscript/cppbc.sgs level.hpp
@popd
