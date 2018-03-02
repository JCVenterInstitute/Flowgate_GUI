package flowgate

class ProtocolSteps {

    Protocol protocol
    String protocolMethodName
    Integer protocolMethodOrder
    Integer protocolParamId

    static constraints = {
    }

    static mapping = {
        protocolMethodName  sqlType: 'varchar(512)'
    }
}
