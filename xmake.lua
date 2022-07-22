set_project("ark_proxy")
set_languages("c17","cxx17")

local vendor = {
    ["boost"]  = "/data/vendor/boost-1.79",
    ["openssl"] = "/data/vendor/openssl-1.1",
    ["gsl"] = os.scriptdir() .. "/vendor/gsl",
    ["date"] = os.scriptdir() .. "/vendor/date",
    ["base64"] = os.scriptdir() .. "/vendor/base64",
    ["vspp"] = os.scriptdir() .. "/vendor/vspp",
}
-- 
option("vendor-boost")
    set_default(vendor["boost"])
    set_showmenu(true)
    after_check(function(option)
        option:add("includedirs","$(vendor-boost)/include", {public = true})
        option:add("linkdirs","$(vendor-boost)/lib")
        option:add("syslinks", "boost_json", "boost_log", "boost_filesystem", "boost_context", "boost_thread", "boost_system")
    end)
--
option("vendor-openssl")
    set_default(vendor["openssl"])
    set_showmenu(true)
    after_check(function(option)
        option:add("includedirs", "$(vendor-openssl)/include", {public = true})
        option:add("linkdirs", "$(vendor-openssl)/lib")
        option:add("syslinks", "ssl", "crypto")
    end)

target("gsl")
    set_kind("headeronly")
    add_headerfiles("vendor/gsl/include/(gsl/**)")
    add_includedirs(vendor["gsl"] .. "/include", {public = true})

target("date")
    set_kind("object")
    set_optimize("faster")
    add_files("vendor/date/src/tz.cpp", {defines = "USE_OS_TZDB"})
    add_includedirs(vendor["date"] .. "/include", {public = true})

target("date-inc")
    set_kind("headeronly")
    add_headerfiles("vendor/date/include/(date/**.h)")

target("base64")
    set_kind("object")
    on_build(function(target)
        local old = os.cd(vendor["base64"])
        if not os.exists("lib/libbase64.o") then
            cprint("${green bright}(vendor) ${clear}build ${magenta bright}base64 ${clear} ...")
            os.runv("make", {}, {envs = {AVX2_CFLAGS = "-mavx2"}})
        end
        os.cd(old)
        table.insert(target:objectfiles(), vendor["base64"] .. "/lib/libbase64.o")
    end)
    on_clean(function(target)
        local old = os.cd("$(vendor-base64)")
        os.runv("make", {"clean"}, {envs = {AVX2_CFLAGS = "-mavx2"}})
        os.cd(old)
    end)
    add_includedirs(vendor["base64"] .. "/include", {public = true})

target("vspp")
    set_kind("headeronly")
    add_headerfiles("vendor/vspp/include/(vspp/**)")
    add_includedirs(vendor["vspp"] .. "/include", {public = true})

target("xbond")
    set_kind("static")
    add_rules("mode.debug", "mode.release", "mode.releasedbg")
    add_options("vendor-boost", "vendor-openssl")
    add_deps("gsl", "date", "base64", "vspp")
    add_defines("BOOST_ASIO_DISABLE_STD_ALIGNED_ALLOC")
    add_cxxflags("-fPIC")
    add_links("pthread")
    add_includedirs("include", {public = true})
    add_files("src/**.cpp")
    add_files("vendor/date/src/tz.cpp", {defines = {"USE_OS_TZDB=1"}})
    add_headerfiles("include/(xbond/**.hpp)")

target("xbond-test")
    set_default(false)
    set_kind("binary")
    add_rules("mode.debug")
    add_options("vendor-boost", "vendor-openssl")
    add_deps("xbond")
    add_defines("BOOST_ASIO_DISABLE_STD_ALIGNED_ALLOC")
    add_includedirs("include")
    add_files("test/**.cpp")
