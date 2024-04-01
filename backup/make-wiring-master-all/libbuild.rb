#!/bin/ruby
require 'fileutils'
VARIANT='standard' #where to get the `pins_arduino.h` file
CPUFREQ="16000000UL"
MCU="atmega644pa"

CFLAGS="-Os -Wl,--gc-sections -ffunction-sections  -fdata-sections"
CPPFLAGS="-Os -Wl,--gc-sections -ffunction-sections  -fdata-sections"
ARFLAGS=""

CC="avr-gcc"
CPP="avr-g++"
AR="avr-ar"


SOURCES="src"
OUTPUTS="output"
OUT = "../../#{OUTPUTS}"
CCFLAGS = "-I#{OUT}/include -I./#{SOURCES}/ -I./ -I./utility/ -I./variants/#{VARIANT} -I./#{OUTPUTS}/include -mmcu=#{MCU} -DF_CPU=#{CPUFREQ}"

require 'io/console'                                                                                                       
def continue_story                                                                                                               
  print "Press Any Key to Continue"                                                                                                    
  STDIN.getch                                                                                                              
  print "            \r" # extra space to overwrite in case next sentence is short                                                                                                              
end        

def getfiles(dir, ending)
  t=""
  Dir["#{dir}/*.#{ending}"].each{ |f|  t+='"'+File.basename(f)+'" '; }
  t
end

def docompile(lib, action)
 cflags = CCFLAGS
 cppflags = cflags
 arflags = ""
if lib == "all"
  Dir.entries("#{SOURCES}/").reject{ |e| e.start_with? '.' }.each { 
  |f| puts f 
  library = f
  dir="#{SOURCES}/#{library}"
` cp template.makefile #{SOURCES}/#{library}/Makefile`
  objs = getfiles(dir, "c").gsub(".c",".o");
  objs += " "+getfiles(dir, "cpp").gsub(".cpp", ".o");
  print "\n"
  hdrs = getfiles(dir, "h")
  domake("#{dir}", action, objs, hdrs, cppflags, cflags, arflags, OUT, "lib#{library}.a") 
  print "\n"
 `rm -f "#{SOURCES}/#{library}/Makefile"`
 }
else
 library = lib
 dir="#{SOURCES}/#{library}"
`cp template.makefile #{SOURCES}/#{library}/Makefile`
 objs = getfiles(dir, "c").gsub(".c",".o");
 objs += " "+getfiles(dir, "cpp").gsub(".cpp", ".o");
 print "\n"
 hdrs = getfiles(dir, "h")
 domake("#{dir}", action, objs, hdrs, cppflags, cflags, arflags, OUT, "lib#{library}.a")
 print "\n"
 `rm -f "#{SOURCES}/#{library}/Makefile"`
end
end

def domake(src, action, objs,hdrs, cppflags, cflags, arflags, out, lib)
   `make -C #{src} #{action} OBJS="#{objs}" CPPFLAGS="#{CPPFLAGS} #{cppflags}" CFLAGS="#{CFLAGS} #{cflags}"\
      ARFLAGS="#{arflags}" OUTPUTS="#{OUT}" HDRS="#{hdrs}" LIBOUT="#{lib}" \
      AR="#{AR}" CC="#{CC}" CPP="#{CPP}" \
      1>&2`
  #make sure to redirect STDOUT to STDERR so it's visible
end

if ARGV.length < 2
  puts 'usage: ./build.rb action library'
  continue_story
  exit
end

#`mkdir -p output/include` #syntax error
#`mkdir -p output/lib`
require 'fileutils'
FileUtils.mkdir_p 'output/include'
FileUtils.mkdir_p 'output/lib'
`cp variants/#{VARIANT}/pins_arduino.h #{OUTPUTS}/include/`

action = ARGV[0] 
library = ARGV[1]

docompile(library, "init")
docompile(library, action)

print "\n"
print "Library Compilation Done"
print "\n"



