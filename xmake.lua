set_project("ark_proxy")
set_languages("c17","cxx17")

if (not vendor) 
then 
    vendor = {}
end
option("vendor-boost")
    set_default(vendor["boost"] or os.getenv("VENDOR-BOOST") or "/data/vendor/boost-1.75")
    set_showmenu(true)
    after_check(function(option)
        option:add("sysincludedirs","$(vendor-boost)/include", {public = true})
        option:add("linkdirs","$(vendor-boost)/lib")
        option:add("syslinks", "boost_json", "boost_filesystem", "boost_context", "boost_system")
    end)
--
option("vendor-openssl")
    set_default(vendor["openssl"] or os.getenv("VENDOR-OPENSSL")  or "/data/vendor/openssl-1.1")
    set_showmenu(true)
    after_check(function(option)
        option:add("sysincludedirs", "$(vendor-openssl)/include", {public = true})
        option:add("linkdirs", "$(vendor-openssl)/lib")
        option:add("syslinks", "ssl", "crypto")
    end)

target("xbond")
    set_kind("static")
    add_rules("mode.debug", "mode.release", "mode.releasedbg")
    add_options("vendor-date", "vendor-boost", "vendor-openssl")
    add_cxxflags("-fPIC")
    add_links("pthread")
    add_includedirs(
        "include",
        "vendor/gsl/include",
        "vendor/date/include",
        {public = true})
    set_pcxxheader("include/xbond/vendor.h")
    add_files("src/**.cpp")
    add_files("vendor/date/src/tz.cpp", {defines = "USE_OS_TZDB"})
    add_headerfiles("include/(xbond/**.h)", "vendor/date/include/(date/**.h)","vendor/gsl/include/(gsl/**)")

target("xbond-test")
    set_default(false)
    set_kind("binary")
    add_rules("mode.debug")
    add_deps("xbond")
    add_includedirs("include")
    set_pcxxheader("include/xbond/vendor.h")
    add_files("test/**.cpp")
