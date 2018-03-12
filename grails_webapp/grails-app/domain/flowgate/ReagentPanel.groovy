package flowgate

class ReagentPanel {

//    static belongsTo = [ExpFile]

    String reagentPanelName

    static constraints = {
        reagentPanelName blank: true
    }

    static mapping ={
        reagentPanelName sqlType: 'varchar(512)'
    }
}
