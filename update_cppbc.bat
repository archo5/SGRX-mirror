@pushd src\common
@echo in src/common
sgsvm -p ../../ext/src/sgscript/cppbc.sgs scritem.hpp
sgsvm -p ../../ext/src/sgscript/cppbc.sgs gamegui.hpp
@popd
@pushd src\tacstrike
@echo in src/tacstrike
sgsvm -p ../../ext/src/sgscript/cppbc.sgs level.hpp
sgsvm -p ../../ext/src/sgscript/cppbc.sgs entities.hpp -i level.hpp
sgsvm -p ../../ext/src/sgscript/cppbc.sgs systems.hpp -i level.hpp
sgsvm -p ../../ext/src/sgscript/cppbc.sgs entities_ts.hpp -i level.hpp
sgsvm -p ../../ext/src/sgscript/cppbc.sgs isr3entities.hpp -i level.hpp
@popd
