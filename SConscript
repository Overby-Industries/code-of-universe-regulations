# SConscript — build libcur inside a consuming SCons project.
#
# For the Aevoria Simulator, with this repo checked out as a submodule at
# `cur/`, the consumer's SConstruct needs three lines:
#
#     env.Append(CPPPATH=["cur/include"])
#     cur_sources = SConscript("cur/SConscript", exports="env")
#     sources = Glob("src/*.cpp") + Glob("core/*.cpp") + cur_sources
#
# It returns the source node list rather than building a library, so libcur
# objects land in the consumer's own shared library and inherit its flags —
# godot-cpp is fussy about mixing runtimes and this sidesteps it entirely.
#
# The source list is kept identical to CMakeLists.txt; adding a file means
# adding it in both places.

Import("env")

cur_sources = [
    File("src/cur_state.cpp"),
    File("src/cur_event.cpp"),
    File("src/cur_entity.cpp"),
    File("src/cur_regulation.cpp"),
    File("src/cur_violation.cpp"),
    File("src/cur_event_log.cpp"),
    File("src/cur_capture_index.cpp"),
    File("src/cur_state_machine.cpp"),
]

# MSVC reads source as the local codepage unless told otherwise, and the
# citations in the transition tables contain section signs. Without this the
# tables compile to mojibake on Windows.
if env.get("is_msvc", False) or env["CC"] == "cl":
    env.AppendUnique(CXXFLAGS=["/utf-8"])

Return("cur_sources")
