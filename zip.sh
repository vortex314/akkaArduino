#rm -rf libraries/microAkka
#mkdir libraries/microAkka
rm $HOME/microAkka.zip
cd ../Common
zip $HOME/microAkka.zip RtosQueue.* Config.* Semaphore.* Log.* Sys.* Uid.* Xdr.* 
cd ../microAkka/src
zip $HOME/microAkka.zip Akka.* Bridge.* Echo.* Sender.* Metric.* Publisher.* System.*
cd ../../akkaArduino
zip $HOME/microAkka.zip readme keywords.txt
unzip $HOME/microAkka.zip -d libraries/microAkka
