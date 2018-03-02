package flowgate

class ReagentPanel {

    String reagentPanelName

    static constraints = {
        reagentPanelName blank: true
    }

    static mapping ={
        reagentPanelName sqlType: 'varchar(512)'
    }
}
