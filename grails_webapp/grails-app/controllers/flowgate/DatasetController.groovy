package flowgate

class DatasetController {

  def selAllFcs(Experiment experiment){
    def fSels = experiment.expFiles*.id
//        println "selall experiment ${experiment}; sels ${fSels} "
    session.fSels = fSels
//        redirect action: 'annotation', id: Experiment.get(1).id, sels: sels, model: [sels: sels]
    redirect action: 'ds_edit', id: experiment.id
  }

  def deselAllFcs(Experiment experiment){
    println "desel experiment ${experiment}"
    session.fSels = []
//        redirect action: 'annotation', id: Experiment.get(1).id, model: [sels: []]
    redirect action: 'ds_edit', id: experiment.id
  }

  def axAddDs(){
    Experiment experiment = Experiment.get(params.eId.toLong())
    params.dsMode = 'create'
    render(contentType: 'text/json') {
      success true
      dsField "${g.render(template: 'datasetTmpl/datasetFieldCreate', model: [experiment: experiment, dsMode:'dsCreate'])}"
    }
  }


  def index() {}



  def ds_edit(Experiment experiment) {
    render view: 'ds_edit', model: [experiment: experiment, dsId: params.dsId], params: params
  }


  def dsUpdate(Experiment experiment){

    println "dataset update"
    params.dsMode = 'edit'
    redirect action: 'ds_edit', id: experiment.id, params: params
  }

}
