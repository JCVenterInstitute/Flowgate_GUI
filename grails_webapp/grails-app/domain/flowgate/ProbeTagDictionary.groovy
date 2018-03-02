package flowgate

class ProbeTagDictionary {

    static hasMany = [probes: Probe]
    String category

    static constraints = {

        category inList: ['Fluorescence', 'Isotope', 'Chromogen']

    }
}
