package flowgate

import grails.databinding.BindingFormat
import grails.gorm.DetachedCriteria
import org.apache.commons.lang3.math.NumberUtils

class Analysis implements Serializable {

//  private static final long serialVersionUID = 1

  static belongsTo = [experiment: Experiment]

  Dataset ds
  Module module
  String analysisName
  String analysisDescription
  User user
  @BindingFormat('yyyy-MM-dd hh:mm:ss')
  Date timestamp
  @BindingFormat('yyyy-MM-dd hh:mm:ss')
  Date dateCreated
  @BindingFormat('yyyy-MM-dd hh:mm:ss')
  Date dateCompleted
  Integer analysisStatus
  String jobNumber
  // TODO check different types! currently using the result file path/name to render in the modal
  // TODO current default resultReportFileName = Reports/AutoReport.html; remove after testing
  String renderResult = 'Reports/AutoReport.html'
  Long dsVersion

  static constraints = {
    analysisName blank: false
    analysisDescription nullable: true, blank: true, blankable: true
    jobNumber nullable: true, blank: true, blankable: true
    renderResult blank: true, nullable: true
    dateCompleted blank: true, nullable: true
  }

  static mapping = {
    analysisName    sqlType: 'varchar(512)'
    analysisDescription  sqlType: 'varchar(1024)'
  }

 def getDataset() {
    if (dsVersion == ds?.version?.toLong()) {
        return this?.ds
    } else {
      if(ds){
        return DatasetHistory.findByDidAndVersion(this.ds.id, this.dsVersion)
      }
    }
  }

  private static DetachedCriteria criteriaForDsHist(long dsId, long dsVersion) {
    DatasetHistory.where {
      did == dsId && version == dsVersion
    }
  }

  def isFailedOnSubmit() {
    return NumberUtils.isNumber(jobNumber) && -1 == Integer.parseInt(jobNumber)
  }

  /**
   * Defines the different Analysis Status.
   */
  public enum Status {

    /**
     * Analysis failed
     */
    DELETED(-2),

    /**
     * Analysis failed
     */
    FAILED(-1),

    /**
     * Analysis processing
     */
    PROCESSING(2),

    /**
     * Analysis completed
     */
    FINISHED(3),

    /**
     * Analysis completed but report file is missing
     */
    REPORT_FILE_MISSING(4);

    private final int statusValue;

    private Status(final int statusValue) {
      this.statusValue = statusValue;
    }

    public int value() {
      return statusValue;
    }
  }
}
