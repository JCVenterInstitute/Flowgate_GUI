package flowgate

import grails.transaction.Transactional
import java.security.MessageDigest
import java.io.*

@Transactional
class ChkSumService {


/**
 * Determine the MD5 or SHA1Sum of a file:
 *
 *   println Checksum.getMD5Sum(new File("file.bin"))
 *   println Checksum.getSHA1Sum(new File("file.bin"))
 *
 * @author: Marcel Maatkamp (m.maatkamp avec gmail dot com)
 */
//    https://gist.github.com/marcelmaatkamp/347630
//    public class Checksum {

        static getSha1sum(file) {
            return getChecksum(file, "SHA1")
        }

        static getMD5sum(file) {
            return getChecksum(file, "MD5")
        }

        static getChecksum(file, type) {
            def digest = MessageDigest.getInstance(type)
            def inputstream = file.newInputStream()
            def buffer = new byte[16384]
            def len

            while((len=inputstream.read(buffer)) > 0) {
                digest.update(buffer, 0, len)
            }
            def sha1sum = digest.digest()

            def result = ""
            for(byte b : sha1sum) {
                result += toHex(b)
            }
            return result
        }

        private static hexChr(int b) {
            return Integer.toHexString(b & 0xF)
        }

        private static toHex(int b) {
            return hexChr((b & 0xF0) >> 4) + hexChr(b & 0x0F)
        }

//        static def main(args) {
//            def sha1sum = Checksum.getSha1sum(new File(args[0]))
//            println "file($args[0]): sha1sum($sha1sum)"
//        }

    }
//}
