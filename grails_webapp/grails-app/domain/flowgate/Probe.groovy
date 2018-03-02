package flowgate

class Probe {

    static belongsTo = [ProbeTagDictionary]
    ProbeTagDictionary probeTagDictionary
    String probeTag
    String preferredShortName
    Integer excitMax
    Integer emissMax
    String casNo
    String precColor

    static constraints = {
        excitMax nullable: true
        emissMax nullable: true
        casNo nullable: true
        precColor nullable: true
    }
}
