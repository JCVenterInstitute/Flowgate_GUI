package flowgate

class Protocol {

    String protocolName

    static constraints = {
    }

    static mapping = {
        protocolName    sqlType: 'varchar(512)'
    }
}
