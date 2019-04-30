#!mruby
#GR-CITRUS Version 2.41

if(!System.use?('SD'))then
  puts "SD can't use."
  System.exit() 
end

fn = SD.open(0, 'song.txt',2)
if(fn < 0)then
  System.exit
end
doc="song"
SD.write(fn, doc, doc.length)
SD.close(fn)
