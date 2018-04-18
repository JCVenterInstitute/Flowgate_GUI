package flowgate
import grails.converters.JSON
import grails.util.Environment
class BootStrap {
  def init = { servletContext ->
    /* */
  if(Environment.current == Environment.DEVELOPMENT || Environment.current == Environment.PRODUCTION) {
      println 'roles & users'
      def superadminRole = Role.findOrSaveByAuthority('ROLE_Administrator')
        def adminRole = Role.findOrSaveByAuthority('ROLE_Admin')
  def userRole = Role.findOrSaveByAuthority('ROLE_User')
  def newUserRole = Role.findOrSaveByAuthority('ROLE_NewUser')
  def guestRole = Role.findOrSaveByAuthority('ROLE_Guest')
  def editExperiment = Role.findOrSaveByAuthority('ROLE_ExperimentEdit')
  def clickExperiment = Role.findOrSaveByAuthority('ROLE_ExperimentClick')
//                    def addFcs = Role.findOrSaveByAuthority('ROLE_AddFcs')
  assert Role.count() == 7
  println 'pass Role count'

  def superadminUser = User.findOrSaveByUsernameAndPasswordAndEmail('super', 'super', 'super@flowgate.ui')
  def adminUser = User.findOrSaveByUsernameAndPasswordAndEmail('admin', 'admin', 'admin@flowgate.ui')
  def userUser = User.findOrSaveByUsernameAndPasswordAndEmail( 'user', 'user', 'user@flowgate.ui')
  def flowGateUser = User.findOrSaveByUsernameAndPasswordAndEmail('flowGate', 'flowGate', 'flowgate.noreply@gmail.com')
  def testUser = User.findOrSaveByUsernameAndPasswordAndEmail( 'acs', 'acs', 'peter.acs@stanford.edu')
  UserRole.create(superadminUser, superadminRole)
  UserRole.create(adminUser, adminRole)
  UserRole.create(userUser, userRole)
  UserRole.create(userUser, editExperiment)
  UserRole.create(userUser, clickExperiment)
  UserRole.create(flowGateUser, userRole)
//                    UserRole.create(flowGateUser, editExperiment)
//                    UserRole.create(flowGateUser, clickExperiment)
  UserRole.create(testUser, newUserRole)
  assert User.count() == 5
  println 'pass User count'

  println 'projects...'
  def orphanProj = new Project(title: 'orphanProject', description: 'dummy project, experiments get assigned to this project on project erasing to keep experiment data ', isActive: false)
  orphanProj.save(failOnError: true)
  def proj1 = new Project(title: 'Glucocorticoid receptor expression on circulating leukocytes differs between healthy male and female adults',
                          description: 'Introduction: The glucocorticoid receptor (GR) is a key receptor involved in inflammatory responses and is influenced by sex steroids. This study measured GR expression on circulating leukocyte subtypes in males and females.\n' +
                              'Methods: A total of 23 healthy adults (12 female) participated in this study. GR expression was measured in leukocyte subtypes using flow cytometry. Peripheral blood\n' +
                              'mononuclear cell (PBMC) gene expression of GR (NR3C1), GR ß, TGF-ß1 and 2, and glucocorticoid-induced leucine zipper (GILZ) were determined by real-time polymerase chain reaction.\n' +
                              'Results: Leukocyte GR was lower in females, particularly in granulocytes, natural killer cells, and peripheral blood mononuclear cells (p<=0.01). GR protein expression\n' +
                              'was different across leukocyte subtypes, with higher expression in eosinophils compared with granulocytes, T lymphocytes, and natural killer cells (p<0.05). There\n' +
                              'was higher gene expression of GR ß in males (p=0.03).\n' +
                              'Conclusions. This is the first study to identify sexual dimorphism in GR expression in healthy adults using flow cytometry. These results may begin to explain the sexual dimorphism seen in many diseases and sex differences in glucocorticoid responsiveness.\n' +
                              'Study publication:\n' +
                              'https://www.ncbi.nlm.nih.gov/pmc/articles/PMC5471823/',
                          isActive: true).save(failOnError: true)
  def proj2 = new Project(title: 'SDY 180', description: 'Study Subjects and Study Design\n' +
    'The study was approved by the Baylor Research Institute Institutional Review Board at Baylor University Medical Center (Dallas, TX). After obtaining written informed consent, healthy adults, aged 18 to 64 years, were enrolled to receive a single intramuscular dose of 2009–2010 seasonal influenza (Fluzone, Sanofi Pasteur, PA), pneumococcal vaccine (Pneumovax23, Merck, NJ), or placebo (saline). Exclusion criteria were pregnancy, active allergy symptoms, or vaccinations within the previous 2 months. Prior to vaccination, participants had two baseline blood draws (on days −7 and 0, with respect to the day of vaccination; see Tables S1 and S2 for study design). Blood was collected in Tempus blood RNA tubes (Life Technologies) for microarray and acid citrate dextrose tubes (ACD, BD Vacutainer) for whole-blood flow cytometry, CBC, and serum analysis of neutralizing antibodies and cytokines. In addition, capillary blood was collected by finger stick for microarray (see Table S5 for study design). Freshly ficolled PBMC were used for sequential isolation of white blood cell subsets.\n',
    isActive: true).save(failOnError: true)
//  def proj3 = new Project(title: '3rd Project', description: 'Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nullam placerat sit amet dui ut egestas. Sed gravida viverra porttitor. In at finibus ipsum. Curabitur mattis rutrum bibendum. Mauris sit amet cursus felis. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Fusce mattis eros sit amet accumsan lobortis. Sed scelerisque et velit ac elementum. Sed at convallis tortor. Quisque viverra elementum tincidunt. Integer rhoncus egestas dolor, in dapibus turpis aliquam non. ',
//                          isActive: true).save(failOnError: true)
  ProjectUser.create(proj1, adminUser, 'owner')
  ProjectUser.create(proj1, userUser, 'member')
  ProjectUser.create(proj2, adminUser, 'owner')
  ProjectUser.create(proj2, userUser, 'member')
//  ProjectUser.create(proj3, adminUser, 'owner')
  //                  ProjectUser.create(proj3, adminUser, 'member')
//  def exp1 = new Experiment(title: 'exp1', description: 'exp1 desc', project: proj1, isActive: true).save(failOnError: true)
//  def exp2 = new Experiment(title: 'exp2', description: 'exp2 desc', project: proj1, isActive: true).save(failOnError: true)
//  def exp3 = new Experiment(title: 'exp3', description: 'exp3 desc', project: proj2, isActive: true).save(failOnError: true)
  println 'experiments...'
  def exp1 = new Experiment(title: 'Flow cytometry identification of sexual dimorphism in glucocorticoid receptor expression from human leukocytes',
                            description: 'Four panels: combo (GR/CD3/CD14/CD16), CD193 (GR/CD193), CD203 (GR/CD203), CD56 (GR/CD3/CD56)\n' +
                                '23 whole blood samples per panel (11 females (age 21-39) and 12 males (age 24-38)) except for panel CD56, which has 21 samples; 90 samples in total.\n' +
                                'Manual gating strategy identified 7 major cell types from leukocytes:\n' +
                                'T lymphocytes (CD3+), Monocyte (CD14+), Granulocytes (CD16+), NK (CD3-CD56+), NKT (CD3+CD56+), Eosinophils (CD193+), Basophils (CD203+)',
                            project: proj1, isActive: true).save(failOnError: true)
  def exp2 = new Experiment(title: 'Experiment2', description: 'We incubated 200 uL of whole blood with pretitrated monoclonal antibodies for 15 min at room temperature followed by lysis of red blood cells (BD FACS Lyse). Samples were processed within 2 hr after blood draw and acquired on a BD Special Order LSRII flow cytometer. Analysis was performed by using FlowJo software (version 8.8.6, TreeStar, Inc.).',
                            project: proj2, isActive: true).save(failOnError: true)
  def exp3 = new Experiment(title: 'exp3', description: 'Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nullam placerat sit amet dui ut egestas. Sed gravida viverra porttitor. In at finibus ipsum. Curabitur mattis rutrum bibendum. Mauris sit amet cursus felis. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Fusce mattis eros sit amet accumsan lobortis. Sed scelerisque et velit ac elementum. Sed at convallis tortor. Quisque viverra elementum tincidunt. Integer rhoncus egestas dolor, in dapibus turpis aliquam non. ',
                            project: proj2, isActive: true).save(failOnError: true)
  ExperimentUser.create(exp1, adminUser, 'owner')
  ExperimentUser.create(exp1, flowGateUser, 'owner')
  ExperimentUser.create(exp1, userUser, 'member')
  ExperimentUser.create(exp2, adminUser, 'owner')
  ExperimentUser.create(exp2, userUser, 'owner')
  ExperimentUser.create(exp3, adminUser, 'owner')
  ExperimentUser.create(exp3, userUser, 'member')

  def expFile1012 = new ExpFile(experiment: exp1, title: '1012_Combo.fcs', chkSum: 'abc', fileName: '1012_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile1013 = new ExpFile(experiment: exp1, title: '1013_Combo.fcs', chkSum: 'abc', fileName: '1013_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile1014 = new ExpFile(experiment: exp1, title: '1014_Combo.fcs', chkSum: 'abc', fileName: '1014_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile1015 = new ExpFile(experiment: exp1, title: '1015_Combo.fcs', chkSum: 'abc', fileName: '1015_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile1016 = new ExpFile(experiment: exp1, title: '1016_Combo.fcs', chkSum: 'abc', fileName: '1016_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile1017 = new ExpFile(experiment: exp1, title: '1017_Combo.fcs', chkSum: 'abc', fileName: '1017_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile1018 = new ExpFile(experiment: exp1, title: '1018_Combo.fcs', chkSum: 'abc', fileName: '1018_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile1019 = new ExpFile(experiment: exp1, title: '1019_Combo.fcs', chkSum: 'abc', fileName: '1019_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile1020 = new ExpFile(experiment: exp1, title: '1020_Combo.fcs', chkSum: 'abc', fileName: '1020_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile1021 = new ExpFile(experiment: exp1, title: '1021_Combo.fcs', chkSum: 'abc', fileName: '1021_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile1022 = new ExpFile(experiment: exp1, title: '1022_Combo.fcs', chkSum: 'abc', fileName: '1022_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile1023 = new ExpFile(experiment: exp1, title: '1023_Combo.fcs', chkSum: 'abc', fileName: '1023_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile1024 = new ExpFile(experiment: exp1, title: '1024_Combo.fcs', chkSum: 'abc', fileName: '1024_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile1025 = new ExpFile(experiment: exp1, title: '1025_Combo.fcs', chkSum: 'abc', fileName: '1025_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile1029 = new ExpFile(experiment: exp1, title: '1029_Combo.fcs', chkSum: 'abc', fileName: '1029_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile1030 = new ExpFile(experiment: exp1, title: '1030_Combo.fcs', chkSum: 'abc', fileName: '1030_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile1031 = new ExpFile(experiment: exp1, title: '1031_Combo.fcs', chkSum: 'abc', fileName: '1031_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile1032 = new ExpFile(experiment: exp1, title: '1032_Combo.fcs', chkSum: 'abc', fileName: '1032_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile1033 = new ExpFile(experiment: exp1, title: '1033_Combo.fcs', chkSum: 'abc', fileName: '1033_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile1035 = new ExpFile(experiment: exp1, title: '1035_Combo.fcs', chkSum: 'abc', fileName: '1035_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile1036 = new ExpFile(experiment: exp1, title: '1036_Combo.fcs', chkSum: 'abc', fileName: '1036_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile1037 = new ExpFile(experiment: exp1, title: '1037_Combo.fcs', chkSum: 'abc', fileName: '1037_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile1038 = new ExpFile(experiment: exp1, title: '1038_Combo.fcs', chkSum: 'abc', fileName: '1038_Combo.fcs', filePath: '/Users/acs/Projects/flowgate/testData/ds1_combo/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)

  ExpFileMetadata expFileMeta1012_subj = new ExpFileMetadata(expFile: expFile1012, mdKey: 'SubjectId', mdVal: '1012')
  ExpFileMetadata expFileMeta1012_gender = new ExpFileMetadata(expFile: expFile1012, mdKey: 'Gender', mdVal: 'Male')
  ExpFileMetadata expFileMeta1012_age = new ExpFileMetadata(expFile: expFile1012, mdKey: 'Age', mdVal: '35')
  ExpFileMetadata expFileMeta1012_ethn = new ExpFileMetadata(expFile: expFile1012, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1012_race = new ExpFileMetadata(expFile: expFile1012, mdKey: 'Race', mdVal: 'Black or African American')

  ExpFileMetadata expFileMeta1013_subj = new ExpFileMetadata(expFile: expFile1013, mdKey: 'SubjectId', mdVal: '1013')
  ExpFileMetadata expFileMeta1013_gender = new ExpFileMetadata(expFile: expFile1013, mdKey: 'Gender', mdVal: 'Male')
  ExpFileMetadata expFileMeta1013_age = new ExpFileMetadata(expFile: expFile1013, mdKey: 'Age', mdVal: '35')
  ExpFileMetadata expFileMeta1013_ethn = new ExpFileMetadata(expFile: expFile1013, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1013_race = new ExpFileMetadata(expFile: expFile1013, mdKey: 'Race', mdVal: 'White')

  ExpFileMetadata expFileMeta1014_subj = new ExpFileMetadata(expFile: expFile1014, mdKey: 'SubjectId', mdVal: '1014')
  ExpFileMetadata expFileMeta1014_gender = new ExpFileMetadata(expFile: expFile1014, mdKey: 'Gender', mdVal: 'Male')
  ExpFileMetadata expFileMeta1014_age = new ExpFileMetadata(expFile: expFile1014, mdKey: 'Age', mdVal: '38')
  ExpFileMetadata expFileMeta1014_ethn = new ExpFileMetadata(expFile: expFile1014, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1014_race = new ExpFileMetadata(expFile: expFile1014, mdKey: 'Race', mdVal: 'White')

  ExpFileMetadata expFileMeta1015_subj = new ExpFileMetadata(expFile: expFile1015, mdKey: 'SubjectId', mdVal: '1015')
  ExpFileMetadata expFileMeta1015_gender = new ExpFileMetadata(expFile: expFile1015, mdKey: 'Gender', mdVal: 'Male')
  ExpFileMetadata expFileMeta1015_age = new ExpFileMetadata(expFile: expFile1015, mdKey: 'Age', mdVal: '25')
  ExpFileMetadata expFileMeta1015_ethn = new ExpFileMetadata(expFile: expFile1015, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1015_race = new ExpFileMetadata(expFile: expFile1015, mdKey: 'Race', mdVal: 'Asian')

  ExpFileMetadata expFileMeta1016_subj = new ExpFileMetadata(expFile: expFile1016, mdKey: 'SubjectId', mdVal: '1016')
  ExpFileMetadata expFileMeta1016_gender = new ExpFileMetadata(expFile: expFile1016, mdKey: 'Gender', mdVal: 'Male')
  ExpFileMetadata expFileMeta1016_age = new ExpFileMetadata(expFile: expFile1016, mdKey: 'Age', mdVal: '38')
  ExpFileMetadata expFileMeta1016_ethn = new ExpFileMetadata(expFile: expFile1016, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1016_race = new ExpFileMetadata(expFile: expFile1016, mdKey: 'Race', mdVal: 'Asian')

  ExpFileMetadata expFileMeta1017_subj = new ExpFileMetadata(expFile: expFile1017, mdKey: 'SubjectId', mdVal: '1017')
  ExpFileMetadata expFileMeta1017_gender = new ExpFileMetadata(expFile: expFile1017, mdKey: 'Gender', mdVal: 'Male')
  ExpFileMetadata expFileMeta1017_age = new ExpFileMetadata(expFile: expFile1017, mdKey: 'Age', mdVal: '27')
  ExpFileMetadata expFileMeta1017_ethn = new ExpFileMetadata(expFile: expFile1017, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1017_race = new ExpFileMetadata(expFile: expFile1017, mdKey: 'Race', mdVal: 'Asian')

  ExpFileMetadata expFileMeta1018_subj = new ExpFileMetadata(expFile: expFile1018, mdKey: 'SubjectId', mdVal: '1018')
  ExpFileMetadata expFileMeta1018_gender = new ExpFileMetadata(expFile: expFile1018, mdKey: 'Gender', mdVal: 'Male')
  ExpFileMetadata expFileMeta1018_age = new ExpFileMetadata(expFile: expFile1018, mdKey: 'Age', mdVal: '24')
  ExpFileMetadata expFileMeta1018_ethn = new ExpFileMetadata(expFile: expFile1018, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1018_race = new ExpFileMetadata(expFile: expFile1018, mdKey: 'Race', mdVal: 'Asian')

  ExpFileMetadata expFileMeta1019_subj = new ExpFileMetadata(expFile: expFile1019, mdKey: 'SubjectId', mdVal: '1019')
  ExpFileMetadata expFileMeta1019_gender = new ExpFileMetadata(expFile: expFile1019, mdKey: 'Gender', mdVal: 'Male')
  ExpFileMetadata expFileMeta1019_age = new ExpFileMetadata(expFile: expFile1019, mdKey: 'Age', mdVal: '34')
  ExpFileMetadata expFileMeta1019_ethn = new ExpFileMetadata(expFile: expFile1019, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1019_race = new ExpFileMetadata(expFile: expFile1019, mdKey: 'Race', mdVal: 'White')

  ExpFileMetadata expFileMeta1020_subj = new ExpFileMetadata(expFile: expFile1020, mdKey: 'SubjectId', mdVal: '1020')
  ExpFileMetadata expFileMeta1020_gender = new ExpFileMetadata(expFile: expFile1020, mdKey: 'Gender', mdVal: 'Male')
  ExpFileMetadata expFileMeta1020_age = new ExpFileMetadata(expFile: expFile1020, mdKey: 'Age', mdVal: '27')
  ExpFileMetadata expFileMeta1020_ethn = new ExpFileMetadata(expFile: expFile1020, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1020_race = new ExpFileMetadata(expFile: expFile1020, mdKey: 'Race', mdVal: 'White')

  ExpFileMetadata expFileMeta1021_subj = new ExpFileMetadata(expFile: expFile1021, mdKey: 'SubjectId', mdVal: '1021')
  ExpFileMetadata expFileMeta1021_gender = new ExpFileMetadata(expFile: expFile1021, mdKey: 'Gender', mdVal: 'Male')
  ExpFileMetadata expFileMeta1021_age = new ExpFileMetadata(expFile: expFile1021, mdKey: 'Age', mdVal: '31')
  ExpFileMetadata expFileMeta1021_ethn = new ExpFileMetadata(expFile: expFile1021, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1021_race = new ExpFileMetadata(expFile: expFile1021, mdKey: 'Race', mdVal: 'White')

  ExpFileMetadata expFileMeta1022_subj = new ExpFileMetadata(expFile: expFile1022, mdKey: 'SubjectId', mdVal: '1022')
  ExpFileMetadata expFileMeta1022_gender = new ExpFileMetadata(expFile: expFile1022, mdKey: 'Gender', mdVal: 'Male')
  ExpFileMetadata expFileMeta1022_age = new ExpFileMetadata(expFile: expFile1022, mdKey: 'Age', mdVal: '25')
  ExpFileMetadata expFileMeta1022_ethn = new ExpFileMetadata(expFile: expFile1022, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1022_race = new ExpFileMetadata(expFile: expFile1022, mdKey: 'Race', mdVal: 'White')

  ExpFileMetadata expFileMeta1023_subj = new ExpFileMetadata(expFile: expFile1023, mdKey: 'SubjectId', mdVal: '1023')
  ExpFileMetadata expFileMeta1023_gender = new ExpFileMetadata(expFile: expFile1023, mdKey: 'Gender', mdVal: 'Female')
  ExpFileMetadata expFileMeta1023_age = new ExpFileMetadata(expFile: expFile1023, mdKey: 'Age', mdVal: '36')
  ExpFileMetadata expFileMeta1023_ethn = new ExpFileMetadata(expFile: expFile1023, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1023_race = new ExpFileMetadata(expFile: expFile1023, mdKey: 'Race', mdVal: 'Asian')

  ExpFileMetadata expFileMeta1024_subj = new ExpFileMetadata(expFile: expFile1024, mdKey: 'SubjectId', mdVal: '1024')
  ExpFileMetadata expFileMeta1024_gender = new ExpFileMetadata(expFile: expFile1024, mdKey: 'Gender', mdVal: 'Female')
  ExpFileMetadata expFileMeta1024_age = new ExpFileMetadata(expFile: expFile1024, mdKey: 'Age', mdVal: '21')
  ExpFileMetadata expFileMeta1024_ethn = new ExpFileMetadata(expFile: expFile1024, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1024_race = new ExpFileMetadata(expFile: expFile1024, mdKey: 'Race', mdVal: 'Asian')

  ExpFileMetadata expFileMeta1025_subj = new ExpFileMetadata(expFile: expFile1025, mdKey: 'SubjectId', mdVal: '1025')
  ExpFileMetadata expFileMeta1025_gender = new ExpFileMetadata(expFile: expFile1025, mdKey: 'Gender', mdVal: 'Female')
  ExpFileMetadata expFileMeta1025_age = new ExpFileMetadata(expFile: expFile1025, mdKey: 'Age', mdVal: '27')
  ExpFileMetadata expFileMeta1025_ethn = new ExpFileMetadata(expFile: expFile1025, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1025_race = new ExpFileMetadata(expFile: expFile1025, mdKey: 'Race', mdVal: 'Asian')

  ExpFileMetadata expFileMeta1029_subj = new ExpFileMetadata(expFile: expFile1029, mdKey: 'SubjectId', mdVal: '1029')
  ExpFileMetadata expFileMeta1029_gender = new ExpFileMetadata(expFile: expFile1029, mdKey: 'Gender', mdVal: 'Female')
  ExpFileMetadata expFileMeta1029_age = new ExpFileMetadata(expFile: expFile1029, mdKey: 'Age', mdVal: '23')
  ExpFileMetadata expFileMeta1029_ethn = new ExpFileMetadata(expFile: expFile1029, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1029_race = new ExpFileMetadata(expFile: expFile1029, mdKey: 'Race', mdVal: 'Asian')

  ExpFileMetadata expFileMeta1030_subj = new ExpFileMetadata(expFile: expFile1030, mdKey: 'SubjectId', mdVal: '1030')
  ExpFileMetadata expFileMeta1030_gender = new ExpFileMetadata(expFile: expFile1030, mdKey: 'Gender', mdVal: 'Female')
  ExpFileMetadata expFileMeta1030_age = new ExpFileMetadata(expFile: expFile1030, mdKey: 'Age', mdVal: '31')
  ExpFileMetadata expFileMeta1030_ethn = new ExpFileMetadata(expFile: expFile1030, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1030_race = new ExpFileMetadata(expFile: expFile1030, mdKey: 'Race', mdVal: 'White')

  ExpFileMetadata expFileMeta1031_subj = new ExpFileMetadata(expFile: expFile1031, mdKey: 'SubjectId', mdVal: '1031')
  ExpFileMetadata expFileMeta1031_gender = new ExpFileMetadata(expFile: expFile1031, mdKey: 'Gender', mdVal: 'Female')
  ExpFileMetadata expFileMeta1031_age = new ExpFileMetadata(expFile: expFile1031, mdKey: 'Age', mdVal: '31')
  ExpFileMetadata expFileMeta1031_ethn = new ExpFileMetadata(expFile: expFile1031, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1031_race = new ExpFileMetadata(expFile: expFile1031, mdKey: 'Race', mdVal: 'Asian')

  ExpFileMetadata expFileMeta1032_subj = new ExpFileMetadata(expFile: expFile1032, mdKey: 'SubjectId', mdVal: '1032')
  ExpFileMetadata expFileMeta1032_gender = new ExpFileMetadata(expFile: expFile1032, mdKey: 'Gender', mdVal: 'Male')
  ExpFileMetadata expFileMeta1032_age = new ExpFileMetadata(expFile: expFile1032, mdKey: 'Age', mdVal: '22')
  ExpFileMetadata expFileMeta1032_ethn = new ExpFileMetadata(expFile: expFile1032, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1032_race = new ExpFileMetadata(expFile: expFile1032, mdKey: 'Race', mdVal: 'Asian')

  ExpFileMetadata expFileMeta1033_subj = new ExpFileMetadata(expFile: expFile1033, mdKey: 'SubjectId', mdVal: '1033')
  ExpFileMetadata expFileMeta1033_gender = new ExpFileMetadata(expFile: expFile1033, mdKey: 'Gender', mdVal: 'Female')
  ExpFileMetadata expFileMeta1033_age = new ExpFileMetadata(expFile: expFile1033, mdKey: 'Age', mdVal: '21')
  ExpFileMetadata expFileMeta1033_ethn = new ExpFileMetadata(expFile: expFile1033, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1033_race = new ExpFileMetadata(expFile: expFile1033, mdKey: 'Race', mdVal: 'Asian')

  ExpFileMetadata expFileMeta1035_subj = new ExpFileMetadata(expFile: expFile1035, mdKey: 'SubjectId', mdVal: '1035')
  ExpFileMetadata expFileMeta1035_gender = new ExpFileMetadata(expFile: expFile1035, mdKey: 'Gender', mdVal: 'Female')
  ExpFileMetadata expFileMeta1035_age = new ExpFileMetadata(expFile: expFile1035, mdKey: 'Age', mdVal: '38')
  ExpFileMetadata expFileMeta1035_ethn = new ExpFileMetadata(expFile: expFile1035, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1035_race = new ExpFileMetadata(expFile: expFile1035, mdKey: 'Race', mdVal: 'White')

  ExpFileMetadata expFileMeta1036_subj = new ExpFileMetadata(expFile: expFile1036, mdKey: 'SubjectId', mdVal: '1036')
  ExpFileMetadata expFileMeta1036_gender = new ExpFileMetadata(expFile: expFile1036, mdKey: 'Gender', mdVal: 'Female')
  ExpFileMetadata expFileMeta1036_age = new ExpFileMetadata(expFile: expFile1036, mdKey: 'Age', mdVal: '30')
  ExpFileMetadata expFileMeta1036_ethn = new ExpFileMetadata(expFile: expFile1036, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1036_race = new ExpFileMetadata(expFile: expFile1036, mdKey: 'Race', mdVal: 'Asian')

  ExpFileMetadata expFileMeta1037_subj = new ExpFileMetadata(expFile: expFile1037, mdKey: 'SubjectId', mdVal: '1037')
  ExpFileMetadata expFileMeta1037_gender = new ExpFileMetadata(expFile: expFile1037, mdKey: 'Gender', mdVal: 'Male')
  ExpFileMetadata expFileMeta1037_age = new ExpFileMetadata(expFile: expFile1037, mdKey: 'Age', mdVal: '27')
  ExpFileMetadata expFileMeta1037_ethn = new ExpFileMetadata(expFile: expFile1037, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1037_race = new ExpFileMetadata(expFile: expFile1037, mdKey: 'Race', mdVal: 'White')

  ExpFileMetadata expFileMeta1038_subj = new ExpFileMetadata(expFile: expFile1038, mdKey: 'SubjectId', mdVal: '1038')
  ExpFileMetadata expFileMeta1038_gender = new ExpFileMetadata(expFile: expFile1038, mdKey: 'Gender', mdVal: 'Female')
  ExpFileMetadata expFileMeta1038_age = new ExpFileMetadata(expFile: expFile1038, mdKey: 'Age', mdVal: '39')
  ExpFileMetadata expFileMeta1038_ethn = new ExpFileMetadata(expFile: expFile1038, mdKey: 'Ethnicity', mdVal: 'Not Hispanic or Latino')
  ExpFileMetadata expFileMeta1038_race = new ExpFileMetadata(expFile: expFile1038, mdKey: 'Race', mdVal: 'White')

  expFile1012.metaDatas = [expFileMeta1012_subj, expFileMeta1012_gender, expFileMeta1012_age, expFileMeta1012_ethn, expFileMeta1012_race]
  expFile1013.metaDatas = [expFileMeta1013_subj, expFileMeta1013_gender, expFileMeta1013_age, expFileMeta1013_ethn, expFileMeta1013_race]
  expFile1014.metaDatas = [expFileMeta1014_subj, expFileMeta1014_gender, expFileMeta1014_age, expFileMeta1014_ethn, expFileMeta1014_race]
  expFile1015.metaDatas = [expFileMeta1015_subj, expFileMeta1015_gender, expFileMeta1015_age, expFileMeta1015_ethn, expFileMeta1015_race]
  expFile1016.metaDatas = [expFileMeta1016_subj, expFileMeta1016_gender, expFileMeta1016_age, expFileMeta1016_ethn, expFileMeta1016_race]
  expFile1017.metaDatas = [expFileMeta1017_subj, expFileMeta1017_gender, expFileMeta1017_age, expFileMeta1017_ethn, expFileMeta1017_race]
  expFile1018.metaDatas = [expFileMeta1018_subj, expFileMeta1018_gender, expFileMeta1018_age, expFileMeta1018_ethn, expFileMeta1018_race]
  expFile1019.metaDatas = [expFileMeta1019_subj, expFileMeta1019_gender, expFileMeta1019_age, expFileMeta1019_ethn, expFileMeta1019_race]
  expFile1020.metaDatas = [expFileMeta1020_subj, expFileMeta1020_gender, expFileMeta1020_age, expFileMeta1020_ethn, expFileMeta1020_race]
  expFile1021.metaDatas = [expFileMeta1021_subj, expFileMeta1021_gender, expFileMeta1021_age, expFileMeta1021_ethn, expFileMeta1021_race]
  expFile1022.metaDatas = [expFileMeta1022_subj, expFileMeta1022_gender, expFileMeta1022_age, expFileMeta1022_ethn, expFileMeta1022_race]
  expFile1023.metaDatas = [expFileMeta1023_subj, expFileMeta1023_gender, expFileMeta1023_age, expFileMeta1023_ethn, expFileMeta1023_race]
  expFile1024.metaDatas = [expFileMeta1024_subj, expFileMeta1024_gender, expFileMeta1024_age, expFileMeta1024_ethn, expFileMeta1024_race]
  expFile1025.metaDatas = [expFileMeta1025_subj, expFileMeta1025_gender, expFileMeta1025_age, expFileMeta1025_ethn, expFileMeta1025_race]
  expFile1029.metaDatas = [expFileMeta1029_subj, expFileMeta1029_gender, expFileMeta1029_age, expFileMeta1029_ethn, expFileMeta1029_race]
  expFile1030.metaDatas = [expFileMeta1030_subj, expFileMeta1030_gender, expFileMeta1030_age, expFileMeta1030_ethn, expFileMeta1030_race]
  expFile1031.metaDatas = [expFileMeta1031_subj, expFileMeta1031_gender, expFileMeta1031_age, expFileMeta1031_ethn, expFileMeta1031_race]
  expFile1032.metaDatas = [expFileMeta1032_subj, expFileMeta1032_gender, expFileMeta1032_age, expFileMeta1032_ethn, expFileMeta1032_race]
  expFile1033.metaDatas = [expFileMeta1033_subj, expFileMeta1033_gender, expFileMeta1033_age, expFileMeta1033_ethn, expFileMeta1033_race]
  expFile1035.metaDatas = [expFileMeta1035_subj, expFileMeta1035_gender, expFileMeta1035_age, expFileMeta1035_ethn, expFileMeta1035_race]
  expFile1036.metaDatas = [expFileMeta1036_subj, expFileMeta1036_gender, expFileMeta1036_age, expFileMeta1036_ethn, expFileMeta1036_race]
  expFile1037.metaDatas = [expFileMeta1037_subj, expFileMeta1037_gender, expFileMeta1037_age, expFileMeta1037_ethn, expFileMeta1037_race]
  expFile1038.metaDatas = [expFileMeta1038_subj, expFileMeta1038_gender, expFileMeta1038_age, expFileMeta1038_ethn, expFileMeta1038_race]

  expFile1012.save()
  expFile1013.save()
  expFile1014.save()
  expFile1015.save()
  expFile1016.save()
  expFile1017.save()
  expFile1018.save()
  expFile1019.save()
  expFile1020.save()
  expFile1021.save()
  expFile1022.save()
  expFile1023.save()
  expFile1024.save()
  expFile1025.save()
  expFile1029.save()
  expFile1030.save()
  expFile1031.save()
  expFile1032.save()
  expFile1033.save()
  expFile1035.save()
  expFile1036.save()
  expFile1037.save()
  expFile1038.save()


  def expFile11 = new ExpFile(experiment: exp2, title: '3.txt', chkSum: 'abc', fileName: '3.txt', filePath: '/Users/acs/Projects/flowgate/testData/dafi/PreprocessedTest/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile21 = new ExpFile(experiment: exp2, title: '5.txt', chkSum: 'cde', fileName: '5.txt', filePath: '/Users/acs/Projects/flowgate/testData/dafi/PreprocessedTest/', createdBy: userUser, reagentPanel: []).save(failOnError: true)
  def expFile31 = new ExpFile(experiment: exp3, title: 'test File', chkSum: 'a41a0f25bcb0454ab43a4451ac59bd6b', fileName: 'test.fcs', filePath: '/Users/acs/Projects/flowgate/testData/flock/', createdBy: adminUser, isActive: true, reagentPanel: []).save(failOnError: true)
  def expFile32 = new ExpFile(experiment: exp3, title: 'fcsFile2.fcs', chkSum: 'efb1fede93ca087a90693353d3cfb2e9276925fd', fileName: 'fcsFile2.fcs', filePath: '/Users/acs/Projects/flowgate/testData/flock/', createdBy: userUser, reagentPanel: []).save(failOnError: true)
  def expFile33 = new ExpFile(experiment: exp3, title: 'fcsFile3.fcs', chkSum: 'ACS', fileName: 'fcsFile3.fcs', filePath: '/Users/acs/Projects/flowgate/testData/flock/', createdBy: adminUser, reagentPanel: []).save(failOnError: true)
//  assert ExpFile.count() == 5
  println "expFile.count()=5 == ${ExpFile.count()} ? passed!"


  ExpFileMetadata expFileMeta11 = new ExpFileMetadata(expFile: expFile11, mdKey: 'Gender', mdVal: 'Female')
  ExpFileMetadata expFileMeta21 = new ExpFileMetadata(expFile: expFile21, mdKey: 'Gender', mdVal: 'Female')
  expFile11.metaDatas = [expFileMeta11]
  expFile11.save()
  expFile21.metaDatas = [expFileMeta21]
  expFile21.save()

  exp2.expFiles = [expFile11, expFile21]

  exp2.save()
  exp3.expFiles = [expFile31, expFile32, expFile33]
  exp3.save()


  def expMetadata = fillExperimentMetadata(exp1)

  //  Dataset
  def ds1 = new Dataset(experiment: exp1, expFiles: [expFile1023,expFile1024,expFile1025,expFile1029,expFile1030,expFile1031,expFile1033,expFile1035,expFile1036,expFile1038],
                        filters: [ExperimentMetadataValue.findAllByMdValue('Female')], name: 'Females', description: 'females')
  ds1.save()
//  def ds2 = new Dataset(experiment: exp2, expFiles: [], name: 'test dataset', description: 'testDataset').save()
  def ds2 = new Dataset(experiment: exp1, expFiles: [expFile1012,expFile1013,expFile1014,expFile1015,expFile1016,expFile1017,expFile1018,expFile1019,expFile1020,expFile1021,expFile1022,expFile1032],
      filters: [ExperimentMetadataValue.findAllByMdValue('Male')],name: 'Males', description: 'males').save()
  def ds3 = new Dataset(experiment: exp1, expFiles: [expFile1015,expFile1016,expFile1017,expFile1018,expFile1023,expFile1024,expFile1025,expFile1029,expFile1031,expFile1032,expFile1033,expFile1036],
      filters: [ExperimentMetadataValue.findAllByMdValue('Asian')],name: 'Asian', description: 'asian').save()
  // TODO test age range dataset

  new UserSettings(user: adminUser, projectOpenId: 2, projectEditModeId: 0, experimentOpenId: 0, experimentEditModeId: 0, expFileOpenIds: ([0] as JSON).toString()).save(failOnError: true)
  new UserSettings(user: userUser, projectOpenId: 2, projectEditModeId: 0, experimentOpenId: 0, experimentEditModeId: 0, expFileOpenIds: ([0] as JSON).toString()).save(failOnError: true)
  new UserSettings(user: flowGateUser, projectOpenId: 2, projectEditModeId: 0, experimentOpenId: 0, experimentEditModeId: 0, expFileOpenIds: ([0] as JSON).toString()).save(failOnError: true)
  assert UserSettings.count() == 3
  AnalysisServer as1 = new AnalysisServer(name: 'local', url: 'http://127.0.0.1:8080', userName: 'peterAcs')
  as1.save()
  AnalysisServer as2 = new AnalysisServer(name: 'SDSC', url: 'http://flowgate.sdsc.edu:8080', userName: 'flowGate', userPw: 'flowGate').save()
  AnalysisServer as3 = new AnalysisServer(name: 'MIT', url: 'https://genepattern.broadinstitute.org', userName: 'flowGate', userPw: 'flowGate').save()
  assert AnalysisServer.count() == 3
  println 'pass AnalysisServer count'
  println 'Modules ...'
  Module mod1 = new Module(server: as3, title: 'FLOCK', name: 'ImmPortFLOCK', moduleParams: []).save(failOnSave: true)
//                    ModuleParam mParam1 = new ModuleParam(module: mod1, pBasic: true, pType: "file", pKey: 'Input.file', defaultVal: '/Users/acs/Projects/flowgate/testData/flock/test.txt').save()
  ModuleParam mParam1 = new ModuleParam(module: mod1, pBasic: true, pType: "file", pKey: 'Input.file', defaultVal: '').save()
  ModuleParam mParam2 = new ModuleParam(module: mod1, pBasic: false, pType: "val", pKey: 'Number.of.bins', defaultVal: '0').save()
  ModuleParam mParam3 = new ModuleParam(module: mod1, pBasic: false, pType: "val", pKey: 'Density.threshold', defaultVal: '0').save()
  ModuleParam mParam4 = new ModuleParam(module: mod1, pBasic: false, pType: "val", pKey: 'Maximum.number.of.populations', defaultVal: '30').save()
  mod1.moduleParams = [mParam1, mParam2, mParam3, mParam4]
  mod1.save()
  Module mod2 = new Module(server: as2, title: 'DAFI_Complete_UCI_GR_Report', name: 'urn:lsid:8080.ichang.flowgate.sdsc.edu:genepatternmodules:24:9', moduleParams: []).save(failOnSave: true)
//  Module mod2 = new Module(server: as2, title: 'DAFI_Complete_UCI_GR_Report', name: 'urn:lsid:8080.ichang.flowgate.sdsc.edu:genepatternmodules:26:3', moduleParams: []).save(failOnSave: true)
//                    ModuleParam mParam5 = new ModuleParam(module: mod2, pBasic: true, pType: "file", pKey: 'input.file', defaultVal: '/Users/acs/Projects/flowgate/testData/dafi/dafitest.txt').save()
  ModuleParam mParam5 = new ModuleParam(module: mod2, pBasic: true, pType: "ds", pKey: 'Input.Dir', defaultVal: ds1.id.toString()).save()
//                    ModuleParam mParam6 = new ModuleParam(module: mod2, pBasic: true, pType: "file", pKey: 'config.file', defaultVal: '/Users/acs/Projects/flowgate/testData/dafi/Run02.config').save()
//  ModuleParam mParam6 = new ModuleParam(module: mod2, pBasic: true, pType: "file", pKey: 'config.file', defaultVal: '').save()
//                    ModuleParam mParam7 = new ModuleParam(module: mod2, pBasic: true, pType: "file", pKey: 'rev.config.file', defaultVal: '/Users/acs/Projects/flowgate/testData/dafi/Empty.config').save()
//  ModuleParam mParam7 = new ModuleParam(module: mod2, pBasic: true, pType: "file", pKey: 'rev.config.file', defaultVal: '').save()
  ModuleParam mParam8 = new ModuleParam(module: mod2, pBasic: false, pType: "val", pKey: 'init.cluster.size', defaultVal: '200').save()
  ModuleParam mParam9 = new ModuleParam(module: mod2, pBasic: false, pType: "val", pKey: 're.cluster.size', defaultVal: '500').save()
//  ModuleParam mParam10 = new ModuleParam(module: mod2, pBasic: false, pType: "val", pKey: 'num.cores', defaultVal: '24').save()
//  mod2.moduleParams = [mParam5, mParam6, mParam7, mParam8, mParam9, mParam10]
  mod2.moduleParams = [mParam5, mParam8, mParam9 ]
  mod2.save()
//                  Module mod3 = new Module(server: as1, title: 'Fcs2Txt', name: 'FcsToCsv', moduleParams: []).save(failOnSave: true)
  Module mod3 = new Module(server: as3, title: 'Fcs2Csv', name: 'FcsToCsv', moduleParams: []).save(failOnSave: true)
//                    ModuleParam mParam11 = new ModuleParam(module: mod3, pBasic: true, pType: "file", pKey: 'Input.FCS.data.file', defaultVal: '/Users/acs/Projects/flowgate/testData/flock/test.fcs').save()
  ModuleParam mParam11 = new ModuleParam(module: mod3, pBasic: true, pType: "file", pKey: 'Input.FCS.data.file', defaultVal: '').save()
  ModuleParam mParam12 = new ModuleParam(module: mod3, pBasic: false, pType: "var", pKey: 'Output.CSV.file.name', defaultVal: '<Input.FCS.data.file_basename>.csv').save()
  ModuleParam mParam13 = new ModuleParam(module: mod3, pBasic: false, pType: "var", pKey: 'Use.full.names', defaultVal: 'TRUE').save()
  ModuleParam mParam14 = new ModuleParam(module: mod3, pBasic: false, pType: "var", pKey: 'Output.keywords.file.name', defaultVal: '<Input.FCS.data.file_basename>').save()
  ModuleParam mParam15 = new ModuleParam(module: mod3, pBasic: false, pType: "var", pKey: 'Output.keywords.mode', defaultVal: 'CSV').save()
  ModuleParam mParam16 = new ModuleParam(module: mod3, pBasic: false, pType: "var", pKey: 'Channel.to.scale.conversion', defaultVal: 'TRUE').save()
  ModuleParam mParam17 = new ModuleParam(module: mod3, pBasic: false, pType: "var", pKey: 'Precision', defaultVal: 'FALSE').save()
  mod3.moduleParams = [mParam11, mParam12, mParam13, mParam14, mParam15, mParam16, mParam17]
  mod3.save()

  //name lsid urn:lsid:8080.ichang.flowgate.sdsc.edu:genepatternmodules:20:3
  Module mod4 = new Module(server: as2, title: 'DAFi_Gating_Plotting_Streamlined', name: 'DAFi_Gating_Plotting_Streamlined', moduleParams: []).save(failOnSave: true)

//                    ModuleParam mParam18 = new ModuleParam(module: mod4, pBasic: true, pType: "dir", pKey: 'Input.Dir', defaultVal: '/Users/acs/Projects/flowgate/testData/dafi/PreprocessedTest/').save()
//                    ModuleParam mParam18 = new ModuleParam(module: mod4, pBasic: true, pType: "dir", pKey: 'Input.Dir', defaultVal: '').save()
  ModuleParam mParam18 = new ModuleParam(module: mod4, pBasic: true, pType: "ds", pKey: 'Input.Dir', defaultVal: ds1.id.toString()).save()

//                    ModuleParam mParam19 = new ModuleParam(module: mod4, pBasic: true, pType: "file", pKey: 'config.file', defaultVal: '/Users/acs/Projects/flowgate/testData/dafi/CLL_new.config').save()
  ModuleParam mParam19 = new ModuleParam(module: mod4, pBasic: true, pType: "file", pKey: 'config.file', defaultVal: '').save()
//                    ModuleParam mParam20 = new ModuleParam(module: mod4, pBasic: true, pType: "file", pKey: 'rev.config.file', defaultVal: '/Users/acs/Projects/flowgate/testData/dafi/Empty.config').save()
  ModuleParam mParam20 = new ModuleParam(module: mod4, pBasic: true, pType: "file", pKey: 'rev.config.file', defaultVal: '').save()
  ModuleParam mParam21 = new ModuleParam(module: mod4, pBasic: true, pType: "var", pKey: 'init.cluster.size', defaultVal: '200').save()
  ModuleParam mParam22 = new ModuleParam(module: mod4, pBasic: true, pType: "var", pKey: 're.cluster.size', defaultVal: '500').save()
  ModuleParam mParam23 = new ModuleParam(module: mod4, pBasic: true, pType: "var", pKey: 'size.param', defaultVal: '1').save()
  ModuleParam mParam24 = new ModuleParam(module: mod4, pBasic: false, pType: "var", pKey: 'axisLabels.size', defaultVal: '14').save()
  ModuleParam mParam25 = new ModuleParam(module: mod4, pBasic: false, pType: "var", pKey: 'PopulationLabel.size', defaultVal: '10').save()
  ModuleParam mParam26 = new ModuleParam(module: mod4, pBasic: false, pType: "var", pKey: 'title.set', defaultVal: '').save()
  ModuleParam mParam27 = new ModuleParam(module: mod4, pBasic: false, pType: "var", pKey: 'Multigates.set', defaultVal: '').save()
  ModuleParam mParam28 = new ModuleParam(module: mod4, pBasic: false, pType: "var", pKey: 'Showparent.set', defaultVal: '').save()
  ModuleParam mParam29 = new ModuleParam(module: mod4, pBasic: false, pType: "var", pKey: 'Sort.set', defaultVal: '').save()
  ModuleParam mParam30 = new ModuleParam(module: mod4, pBasic: false, pType: "var", pKey: 'Showgrid.set', defaultVal: '').save()
  ModuleParam mParam31 = new ModuleParam(module: mod4, pBasic: false, pType: "var", pKey: 'Hidegatelines.set', defaultVal: '').save()
//            mod3.moduleParams = [mParam11, mParam12, mParam13, mParam14, mParam15, mParam16, mParam17]
  mod4.save()

//  Module mod5 = new Module(server: as2, title: 'Multi_DAFI_Complete_UCI_GR_Report_wC', name: 'urn:lsid:8080.ichang.flowgate.sdsc.edu:genepatternmodules:26:3', moduleParams: []).save(failOnSave: true)
  Module mod5 = new Module(server: as2, title: 'Multi_DAFi_Complete_UCI_GR_Report_Metadata', name: 'urn:lsid:8080.ichang.flowgate.sdsc.edu:genepatternmodules:30:4', moduleParams: []).save(failOnSave: true)
  ModuleParam m5mP1 = new ModuleParam(module: mod5, pBasic: true, pType: "ds", pKey: 'Input.Dir', defaultVal: ds1.id.toString()).save()
  ModuleParam m5mP2 = new ModuleParam(module: mod5, pBasic: false, pType: "val", pKey: 'init.cluster.size', defaultVal: '200').save()
  ModuleParam m5mP3 = new ModuleParam(module: mod5, pBasic: false, pType: "val", pKey: 're.cluster.size', defaultVal: '500').save()
  ModuleParam m5mP4 = new ModuleParam(module: mod5, pBasic: false, pType: "field", pKey: 'Input.Desc', defaultVal: 'description.txt').save()
  ModuleParam m5mP5 = new ModuleParam(module: mod5, pBasic: false, pType: "meta", pKey: 'Input.Meta', defaultVal: 'metadata.txt').save()
  mod5.moduleParams = [ m5mP1, m5mP2, m5mP3, m5mP4, m5mP5 ]
  mod5.save()

  assert Module.count() == 5
  println 'pass Module count'
//  assert ModuleParam.count() ==
  println 'pass ModuleParams count'

  Analysis an0 = new Analysis(analysisName: 'Identification of sexual dimorphism in glucocorticoid receptor expression by computational analysis of flow cytometry data',
                              analysisDescription: 'Step 1: Quality control and preprocessing, including data transformation and re-compensation\n' +
                                  'Step 2: Computational identification of cell populations using DAFi and DAFi + FLOCK\n' +
                                  'Step 3: MFI and proportion statistics from cross-sample comparison',
                              user: userUser,
                              timestamp: new Date(117,11,17,11,30,00),
                              analysisStatus: 3,
                              jobNumber: 1292,
                              analysisServer: 'localhost', analysisUser: 'flowGate', analysisUserPw: 'flowGate',
                              experiment: exp1, module: mod4).save()
  assert Analysis.count() == 1
  println 'pass Analysis count'

                    //AnalysisPipeline analysisPipeline = new AnalysisPipeline(experiment: exp1, name: 'Basic Analysis', elements: [])
                    //analysisPipeline.save()
                    //assert AnalysisPipeline.count() == 1
//                    println 'pass AnalysisPipeline count'

        fillFcsKeywords()


        def probes = fillProbes()

        println "Probes count ${probes.size().toString()}"
        println "probe.cnt ${Probe.count().toString()}"

        fillMetaData()
//                    def metadata = fillMetaData()
        println "MetadataVoc.cnt ${MetadataVoc.count().toString()}"


        def i=1
        println 'bootstrapping done!'
      }
    }


    def fillMetaData() {

            MetadataVoc mdv = new MetadataVoc(mdKey: 'Organism', mdVal: '', category: 'Organisms', dispOrder: 0, status: 'active').save()
//            println "mdv ${mdv?.dump()}  e: ${mdv?.errors}"
            new MetadataVoc(mdKey: 'Sample Source', mdVals: '', category: 'Sample Info', dispOrder: 0, status: 'active').save()
            new MetadataVoc(mdKey: 'Age', mdVal: '', category: 'Sample Info', dispOrder: 0, status: 'active').save()
            new MetadataVoc(mdKey: 'Age Unit', mdVal: '', category: 'Sample Info', dispOrder: 0, status: 'active').save()
            new MetadataVoc(mdKey: 'Gender', mdVal: '', category: 'Sample Info', dispOrder: 0, status: 'active').save()
            new MetadataVoc(mdKey: 'Phenotype', mdVal: '', category: 'Sample Info', dispOrder: 0, status: 'active').save()
            new MetadataVoc(mdKey: 'Treatment', mdVal: '', category: 'Sample Info', dispOrder: 0, status: 'active').save()
//            new MetadataVoc(mdKey: 'Staning', mdVal: '', category: 'Sample Info', dispOrder: 0, status: 'active').save()
            new MetadataVoc(mdKey: 'Location', mdVal: '', category: 'Sample Info', dispOrder: 0, status: 'active').save()
            new MetadataVoc(mdKey: 'Condition', mdVal: '', category: 'Sample Info', dispOrder: 0, status: 'active').save()
            new MetadataVoc(mdKey: 'Dose', mdVal: '', category: 'Sample Info', dispOrder: 0, status: 'active').save()
            new MetadataVoc(mdKey: 'Timepoint', mdVal: '', category: 'Sample Info', dispOrder: 0, status: 'active').save()
            new MetadataVoc(mdKey: 'Individual', mdVal: '', category: 'Sample Info', dispOrder: 0, status: 'active').save()
//            new MetadataVoc(mdKey: '', mdVal: '', category: '', dispOrder: 0, status: 'active').save()
//            new MetadataVoc(mdKey: '', mdVal: '', category: '', dispOrder: 0, status: 'active').save()
//            new MetadataVoc(mdKey: '', mdVal: '', category: '', dispOrder: 0, status: 'active').save()
//            new MetadataVoc(mdKey: '', mdVal: '', category: '', dispOrder: 0, status: 'active').save()
//            new MetadataVoc(mdKey: '', mdVal: '', category: '', dispOrder: 0, status: 'active').save()
//            new MetadataVoc(mdKey: '', mdVal: '', category: '', dispOrder: 0, status: 'active').save()
//            new MetadataVoc(mdKey: '', mdVal: '', category: '', dispOrder: 0, status: 'active').save()
//            new MetadataVoc(mdKey: '', mdVal: '', category: '', dispOrder: 0, status: 'active').save()
//            new MetadataVoc(mdKey: '', mdVal: '', category: '', dispOrder: 0, status: 'active').save()
//            new MetadataVoc(mdKey: '', mdVal: '', category: '', dispOrder: 0, status: 'active').save()
//            new MetadataVoc(mdKey: '', mdVal: '', category: '', dispOrder: 0, status: 'active').save()
//            new MetadataVoc(mdKey: '', mdVal: '', category: '', dispOrder: 0, status: 'active').save()
//            new MetadataVoc(mdKey: '', mdVal: '', category: '', dispOrder: 0, status: 'active').save()
    }

    def fillExperimentMetadata(Experiment exp){

            ExperimentMetadataValue mdValSubj = new ExperimentMetadataValue(mdValue: 'SubjectID', mdType: 'String', dispOrder: 0).save()
            ExperimentMetadataValue mdValGenF = new ExperimentMetadataValue(mdValue: 'Female', mdType: 'String', dispOrder: 2).save()
            ExperimentMetadataValue mdValGenM = new ExperimentMetadataValue(mdValue: 'Male', mdType: 'String', dispOrder: 4).save()
            ExperimentMetadataValue mdValMarker1 = new ExperimentMetadataValue(mdValue: 'CD16', mdType: 'String', dispOrder: 30).save()
            ExperimentMetadataValue mdValMarker2 = new ExperimentMetadataValue(mdValue: 'GR', mdType: 'String', dispOrder: 40).save()
            ExperimentMetadataValue mdValMarker3 = new ExperimentMetadataValue(mdValue: 'not used', mdType: 'String', dispOrder: 0).save()
            ExperimentMetadataValue mdValMarker4 = new ExperimentMetadataValue(mdValue: 'CD14', mdType: 'String', dispOrder: 20).save()
            ExperimentMetadataValue mdValMarker6 = new ExperimentMetadataValue(mdValue: 'CD3', mdType: 'String', dispOrder: 10).save()
            ExperimentMetadataValue mdValReag1 = new ExperimentMetadataValue(mdValue: 'VioBlue-A', mdType: 'String', dispOrder: 40).save()
            ExperimentMetadataValue mdValReag2 = new ExperimentMetadataValue(mdValue: 'FITC-A', mdType: 'String', dispOrder: 20).save()
            ExperimentMetadataValue mdValReag3 = new ExperimentMetadataValue(mdValue: 'PE-A', mdType: 'String', dispOrder: 30).save()
            ExperimentMetadataValue mdValReag4 = new ExperimentMetadataValue(mdValue: 'PI/PE-Cy5.5-A/PECy5/PerCP', mdType: 'String', dispOrder: 38).save()
            ExperimentMetadataValue mdValReag5 = new ExperimentMetadataValue(mdValue: 'PE-Cy7-A', mdType: 'String', dispOrder: 32).save()
            ExperimentMetadataValue mdValReag6 = new ExperimentMetadataValue(mdValue: 'APC-A', mdType: 'String', dispOrder: 0).save()
            ExperimentMetadataValue mdValReag7 = new ExperimentMetadataValue(mdValue: 'APC-Cy7-A', mdType: 'String', dispOrder: 10).save()
            ExperimentMetadataValue mdValChanFl1A = new ExperimentMetadataValue(mdValue: 'FL1-A', mdType: 'String', dispOrder: 0).save()
            ExperimentMetadataValue mdValChanFl2A = new ExperimentMetadataValue(mdValue: 'FL2-A', mdType: 'String', dispOrder: 10).save()
            ExperimentMetadataValue mdValChanFl3A = new ExperimentMetadataValue(mdValue: 'FL3-A', mdType: 'String', dispOrder: 20).save()
            ExperimentMetadataValue mdValChanFl4A = new ExperimentMetadataValue(mdValue: 'FL4-A', mdType: 'String', dispOrder: 30).save()
            ExperimentMetadataValue mdValChanFl5A = new ExperimentMetadataValue(mdValue: 'FL5-A', mdType: 'String', dispOrder: 40).save()
            ExperimentMetadataValue mdValChanFl6A = new ExperimentMetadataValue(mdValue: 'FL6-A', mdType: 'String', dispOrder: 50).save()
            ExperimentMetadataValue mdValChanFl7A = new ExperimentMetadataValue(mdValue: 'FL7-A', mdType: 'String', dispOrder: 60).save()

            ExperimentMetadataValue mdValEthn1 = new ExperimentMetadataValue(mdValue: 'Not Hispanic or Latino', mdType: 'String', dispOrder: 0).save()

            ExperimentMetadataValue mdValRace1 = new ExperimentMetadataValue(mdValue: 'Black or African American', mdType: 'String', dispOrder: 0).save()
            ExperimentMetadataValue mdValRace2 = new ExperimentMetadataValue(mdValue: 'White', mdType: 'String', dispOrder: 10).save()
            ExperimentMetadataValue mdValRace3 = new ExperimentMetadataValue(mdValue: 'Asian', mdType: 'String', dispOrder: 20).save()

            new ExperimentMetadata(experiment: exp, mdKey: 'SubjectID', mdVals: [], mdCategory: 'Demographics', dispOrder: 02, expFiles: []).save()
            new ExperimentMetadata(experiment: exp, mdKey: 'Age', mdVals: [], mdCategory: 'Demographics', dispOrder: 100, expFiles: []).save()
            new ExperimentMetadata(experiment: exp, mdKey: 'Gender', mdVals: [mdValGenF,mdValGenM], mdCategory: 'Demographics', dispOrder: 04, expFiles: []).save()
            new ExperimentMetadata(experiment: exp, mdKey: 'Ethnicity', mdVals: [mdValEthn1], mdCategory: 'Demographics', dispOrder: 20, expFiles: []).save()
            new ExperimentMetadata(experiment: exp, mdKey: 'Race', mdVals: [mdValRace1,mdValRace2,mdValRace3], mdCategory: 'Demographics', dispOrder: 40, expFiles: []).save()
            new ExperimentMetadata(experiment: exp, mdKey: 'Fluorochrome', mdVals: [mdValReag1,mdValReag2,mdValReag3,mdValReag4,mdValReag5,mdValReag6,mdValReag7], mdCategory: 'Reagents', dispOrder: 1, expFiles: []).save()
            def em1 = new ExperimentMetadata(experiment: exp, mdKey: 'Marker', mdVals: [mdValMarker1,mdValMarker2,mdValMarker3,mdValMarker4,mdValMarker3,mdValMarker6,mdValMarker3], mdCategory: 'Reagents', dispOrder: 2, expFiles: []).save()
            new ExperimentMetadata(experiment: exp, mdKey: 'Channel', mdVals: [mdValChanFl1A,mdValChanFl2A,mdValChanFl3A,mdValChanFl4A,mdValChanFl5A,mdValChanFl6A,mdValChanFl7A], mdCategory: 'Reagents', dispOrder: 0, expFiles: []).save()

            println "ExperimentMetadata.cnt ${ExperimentMetadata.count()}"
    }


    //******************************************************************************************************************

    def fillFcsKeywords(){
            //            safe fcs keywords
//            # parameters
            new FcsKeywords(keyName: '#ACQUISITIONTIMEMILLI', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '#ATIM', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '#CFLOWCAPTUREDDATE', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '#CFLOWDECADESn', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '#CFLOWnCOLORCOMP', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '#LASERCONFIGURATION', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '#PnMaxUsefulChannelDNA', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '#PnMaxUsefulDataChannel', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '#PnVirtualGain', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '#SPACERS', comment: '', active: true, safe: true, flowRepo: true).save()
//            $ parameters
            new FcsKeywords(keyName: '$BEGINANALYSIS', comment: 'analysisStart', active: true, safe: true, flowRepo: true).save() // Integer
            new FcsKeywords(keyName: '$BEGINDATA', comment: 'dataStart', active: true, safe: true, flowRepo: true).save() // Integer
            new FcsKeywords(keyName: '$BEGINSTEXT', comment: 'supplementalStart', active: true, safe: true, flowRepo: true).save() // Integer
            new FcsKeywords(keyName: '$BTIM', comment: 'bTime', active: true, safe: true, flowRepo: true).save() // ------
            new FcsKeywords(keyName: '$BYTEORD', comment: 'byteOrder', active: true, safe: true, flowRepo: true).save() //influences littleEndianP = bO.equals("1,2,3,4")
            new FcsKeywords(keyName: '$COM', comment: 'comment', active: true, safe: false, flowRepo: false).save() //influences littleEndianP = bO.equals("1,2,3,4")
            new FcsKeywords(keyName: '$COMP', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$CYT', comment: 'cytometer', active: true, safe: true, flowRepo: true).save() // ------
            new FcsKeywords(keyName: '$CYTSN', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$DATATYPE', comment: 'dataType', active: true, safe: true, flowRepo: true).save() // Integer
            new FcsKeywords(keyName: '$DATE', comment: 'experimentDate', active: true, safe: true, flowRepo: true).save() // ------
            new FcsKeywords(keyName: '$DFCiTOj', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$ENDANALYSIS', comment: 'analysisEnd', active: true, safe: true, flowRepo: true).save() // Integer
            new FcsKeywords(keyName: '$ENDANYLYSIS', comment: 'analysisEnd Typo', active: true, safe: true, flowRepo: true).save() // Integer
            new FcsKeywords(keyName: '$ENDDATA', comment: 'dataEnd', active: true, safe: true, flowRepo: true).save() //Integer
            new FcsKeywords(keyName: '$ENDSTEXT', comment: 'supplementalEnd endsText', active: true, safe: true, flowRepo: true).save() // Integer
            new FcsKeywords(keyName: '$ETIM', comment: 'expTime', active: true, safe: true, flowRepo: true).save() // ------
            new FcsKeywords(keyName: '$ETIME', comment: 'ev expTime Typo', active: true, safe: true, flowRepo: true).save() // ------
            new FcsKeywords(keyName: '$FIL', comment: 'expFile', active: true, safe: false, flowRepo: false).save() // ------
            new FcsKeywords(keyName: '$INST', comment: 'instrument', active: true, safe: false, flowRepo: false).save() // ------
            new FcsKeywords(keyName: '$LOST', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$MODE', comment: 'data mode [(L)ist mode - preferred, (H)istogram - deprecated]', active: true, safe: true, flowRepo: true).save() // ------
            new FcsKeywords(keyName: '$NEXTDATA', comment: 'nextData', active: true, safe: true, flowRepo: true).save() // ------
            new FcsKeywords(keyName: '$OP', comment: 'operatorName', active: true, safe: false, flowRepo: false).save() // ------
            new FcsKeywords(keyName: '$PAR', comment: 'parameters', active: true, safe: true, flowRepo: true).save() // Integer
            new FcsKeywords(keyName: '$PKNn', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$PKn', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$PROJ', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$Pnew Probe(n}B', comment: 'channelBits', active: true, safe: true, flowRepo: true).save() // 8=asci, 16=int, 32=float, 64=double
            new FcsKeywords(keyName: '$PnCALIBRATION', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$PnD', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$Pnew Probe(n}DISPLAY', comment: 'displayString', active: true, safe: true, flowRepo: true).save() // String 'log' ? -> displayLog Boolean
            new FcsKeywords(keyName: '$Pnew Probe(n}E', comment: 'channelAmp', active: true, safe: true, flowRepo: true).save() // ------
            new FcsKeywords(keyName: '$PnF', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$Pnew Probe(n}G', comment: 'channelGain', active: true, safe: true, flowRepo: true).save() // String[]
            new FcsKeywords(keyName: '$PnL', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$Pnew Probe(n}N', comment: 'channelShortname channelName', active: true, safe: true, flowRepo: true).save() // String[]
            new FcsKeywords(keyName: '$PnO', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$PnP', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$Pnew Probe(n}R', comment: 'channelRange', active: true, safe: true, flowRepo: true).save() // Double[]
            new FcsKeywords(keyName: '$Pnew Probe(n}S', comment: 'channelName channelShortname', active: true, safe: true, flowRepo: true).save() // String[]
            new FcsKeywords(keyName: '$PnT', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$Pnew Probe(n}V', comment: 'channelVoltage', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$RUNNUMBER', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$RnI', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$SPILL', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$SPILLOVER', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$SRC', comment: 'source', active: true, safe: false, flowRepo: false).save() // ------
            new FcsKeywords(keyName: '$SYS', comment: 'operatingSystem', active: true, safe: false, flowRepo: false).save() // ------
            new FcsKeywords(keyName: '$TEMPELECTRONICS', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$TEMPOPTICS', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$TIMESTEP', comment: 'timeStep', active: true, safe: true, flowRepo: true).save() // ------
            new FcsKeywords(keyName: '$TOT', comment: 'totalEvents', active: true, safe: true, flowRepo: true).save() // Integer
            new FcsKeywords(keyName: '$TR', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$VOL', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '$WELLID', comment: '', active: true, safe: true, flowRepo: true).save()
//            @ parameters
            new FcsKeywords(keyName: '@ABSCALFACTOR', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@AUX_SIGNAL', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@Acquisition Protocol Offset', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@BASELINEOFFSET', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@BLUELASERPOWER_END', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@BLUELASERPOWER_START', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@BLUELASERSHUTTER', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@BLUETARGETPOWER', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@BUILDNUMBER', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@CAROUSEL', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@COMPENSATIONMODE', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@CRS20BITFORMAT', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@CYTOMETERID', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@DISCRIMINATOR', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@PANEL', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@PnADDRESS', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@PnC', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@PnDIVIDER', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@PnGAIN', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@PnQ', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@PnU', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@PnX', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@PnZ', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@RATIODENOMINATORMUX', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@RATIONUMERATORMUX', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@RATIO_DENOMINATOR', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@RATIO_NUMERATOR', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@REDLASERPOWER_END', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@REDLASERPOWER_START', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@REDLASERSHUTTER', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@REDTARGETPOWER', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@RESAVEDFILE', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@STOPREASON', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@TUBENO', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@VIOLETLASERPOWER_END', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@VIOLETLASERPOWER_START', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@VIOLETLASERSHUTTER', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@VIOLETTARGETPOWER', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: '@Y2KDATE', comment: '', active: true, safe: true, flowRepo: true).save()
//            BD$ parameters
            new FcsKeywords(keyName: 'BD$AcqLibVersion', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'BD$LASERMODE', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'BD$NPAR', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'BD$PnN', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'BD$WORDn', comment: '', active: true, safe: true, flowRepo: true).save()
//            flowcore_$ parameters
            new FcsKeywords(keyName: 'flowCore_$PnR', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'flowCore_$PnRmax', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'flowCore_$PnRmin', comment: '', active: true, safe: true, flowRepo: true).save()
//            GTI$ parameters
            new FcsKeywords(keyName: 'GTI$ANALYSISDATE', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$ANALYSISSETUPBEGIN', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$ANALYSISSETUPEND', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$ANALYSISTIME', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$AREASCALE', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$ASSAYTYPE', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$BEGINLOG', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$ENDLOG', comment: '', active: true, safe: true, flowRepo: true).save()    //  not alphabetically
            new FcsKeywords(keyName: 'GTI$CHANxxx', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$COMPENSATIONxxx', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$DILUTIONFACTOR', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$ERRORCOUNT', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$FLOWRATECAL', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$Gxxx', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$HIGHCONCTRIGGER', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$INSTRUMENTTYPE', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$NGATES', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$NREGIONS', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$NUMPLOTS', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$ORIGINALVOLUME', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$PARAMxxx', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$PLOTxxx', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$PMTxxx', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$PUMPSAMPLESPEED', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$QUANTITY', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$REFRESHRATE', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$Rxxx', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$SPILLOVERUUID', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$SUBREGIONLIST', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$TERMINATIONCOUNT', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$TIMESCALE', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$TOTALNUMREADINGS', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$WELL', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GTI$WIDTHSCALE', comment: '', active: true, safe: true, flowRepo: true).save()
//            P$ parameters
            new FcsKeywords(keyName: 'P$ACQTIMES', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'P$CELLCONC', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'P$CNTSENO', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'P$COUNTVOLUME', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'P$PLLn', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'P$PMODEn', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'P$PULn', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'P$SPEED', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'P$SPEEDIS', comment: '', active: true, safe: true, flowRepo: true).save()
//            no prefix parameters
            new FcsKeywords(keyName: 'ACQTIME', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'ANALOG_COMP', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'APPLICATION', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'APPLY_COMPENSATION', comment: 'applyCompensation Typo', active: true, safe: true, flowRepo: true).save() // Boolean
            new FcsKeywords(keyName: 'APPLY COMPENSATION', comment: 'applyCompensation', active: true, safe: true, flowRepo: true).save() // Boolean
            new FcsKeywords(keyName: 'AUTOBS', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'Acq. Software', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'CREATOR', comment: 'creatorSoftware', active: true, safe: true, flowRepo: true).save() // ------
            new FcsKeywords(keyName: 'CST BASELINE DATE', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'CST BEADS LOT ID', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'CST SETUP DATE', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'CST SETUP STATUS', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'CYTNUM', comment: 'cytometerNumber', active: true, safe: true, flowRepo: true).save() // ------
            new FcsKeywords(keyName: 'CYTOF_DATA_SHIFT', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'CYTOMETER CONFIG CREATE DATE', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'CYTOMETER CONFIG key', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'CalibFile', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'DET_SPILL', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'END BATCH', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'EXPERIMENT NAME', comment: 'experimentName', active: true, safe: false, flowRepo: false).save() // ------
            new FcsKeywords(keyName: 'EXPORT TIME', comment: 'exportTime', active: true, safe: false, flowRepo: false).save() // ------
            new FcsKeywords(keyName: 'EXPORT USER NAME', comment: 'exportUser', active: true, safe: false, flowRepo: false).save() // ------
            new FcsKeywords(keyName: 'START BATCH', comment: '', active: true, safe: true, flowRepo: true).save() //not alphabethically ordered
            new FcsKeywords(keyName: 'FCSversion', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'FIRMWAREVERSION', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'FJ$ACQSTATE', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'FJ_$PnR', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'FJ_$TIMESTEP', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'FJ_FCS_VERSION', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'FSC ASF', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'GUID', comment: 'GUID', active: true, safe: false, flowRepo: false).save() // ------
            new FcsKeywords(keyName: 'LASERnew Probe(n}ASF', comment: 'laserASF[]', active: true, safe: false, flowRepo: false).save() // String[]
            new FcsKeywords(keyName: 'LASERnew Probe(n}DELAY', comment: 'laserDelay[]', active: true, safe: true, flowRepo: true).save() // String[]
            new FcsKeywords(keyName: 'LasernDelay', comment: 'laserDelay[] Typo', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'LASERnew Probe(n}NAME', comment: 'laserName[]', active: true, safe: false, flowRepo: false).save() // String[]
            new FcsKeywords(keyName: 'LASERnPOWER', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'LASERnWAVELENGTH', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'LASERnkey', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'Manufacturer', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'NOZZLEDIAMETER', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'NUMSORTWAYS', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'PANEL key', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'PANEL_key', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'PLATE ID', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'PLATE key', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'PnBS', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'PnCHANNELTYPE', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'PnDISPLAY', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'PnDetectorkey', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'PnHi', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'PnLo', comment: '', active: true, safe: true, flowRepo: true).save()  //      not alphabetically
            new FcsKeywords(keyName: 'PnLASER', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'PnMS', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'PnTHRESVOL', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'PnType', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'QnI', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'QnS', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'QnU', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'SAMPLE ID', comment: 'sampleName', active: true, safe: false, flowRepo: false).save()
            new FcsKeywords(keyName: 'SHEATHPRESSURE', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'SOFTWARE', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'SOFTWAREREVISION', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'SPILL', comment: 'spillString', active: true, safe: true, flowRepo: true).save() // ------
            new FcsKeywords(keyName: 'SPILLOVER', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'TEMPELECTRONICS', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'TEMPOPTICS', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'THRESHOLD', comment: 'threshold', active: true, safe: true, flowRepo: true).save() // ------
            new FcsKeywords(keyName: 'TIMETICKS', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'transformation', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'TUBE key', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'TUBE_key', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'TUBE NAME', comment: '', active: true, safe: false, flowRepo: false).save() // ------
            new FcsKeywords(keyName: 'UTOPEXBUILD', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'VOLTAGECHANGED', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'VSH_APP', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'VSH_APP_VERSION', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'WELL ID', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'WIDTHPARAMUPSHIFT', comment: '', active: true, safe: true, flowRepo: true).save()
            new FcsKeywords(keyName: 'WINDOW EXTENSION', comment: 'windowExtension', active: true, safe: true, flowRepo: true).save() // ------
            new FcsKeywords(keyName: 'WORKSTATION', comment: '', active: true, safe: true, flowRepo: true).save()
//                    assert FcsKeywords.count() == 236
            println FcsKeywords.count().toString()
            println 'passed FcsKeywords count'

    }

    def fillProbes(){
            ProbeTagDictionary fluorescence = new ProbeTagDictionary(category: 'Fluorescence').save()
            ProbeTagDictionary isotope = new ProbeTagDictionary(category: 'Isotope').save()
            ProbeTagDictionary chromogen = new ProbeTagDictionary(category: 'Chromogen').save()
            println "ProbeTagDict.size ${ProbeTagDictionary.count()}"
            def probes = []
            Probe probe
            probe = new Probe(probeTagDictionary: fluorescence, probeTag:"6-Carboxyfluorescein", preferredShortName:"6-FAM", excitMax:492, emissMax:518, casNo:"92557-81-8*")//.save()
            probe.save()
            probes.add(probe)
//            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"6-Carboxyfluorescein", preferredShortName:"6-FAM", excitMax:492, emissMax:518, casNo:"92557-81-8*").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"6-JOE", preferredShortName:"6-JOE", excitMax:520, emissMax:550, casNo:"113394-23-3*" ).save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"7-Aminoactinomycin D", preferredShortName:"7-AAD", excitMax:545, emissMax:647, casNo:"7240-37-1" ).save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Acridine Orange", preferredShortName:"Acrid-O", excitMax:503, emissMax:525, casNo:"65-61-2" ).save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Alexa Fluor 350", preferredShortName:"AF350", excitMax:343, emissMax:442, casNo:"244636-14-4" ).save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Alexa Fluor 405", preferredShortName:"AF405", excitMax:401, emissMax:425, casNo:"791637-08-6").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Alexa Fluor 430", preferredShortName:"AF430", excitMax:433, emissMax:541, casNo:"467233-94-9").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Alexa Fluor 488", preferredShortName:"AF488", excitMax:496, emissMax:519, casNo:"247144-99-6").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Alexa Fluor 500", preferredShortName:"AF500", excitMax:503, emissMax:525, casNo:"798557-08-1").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Alexa Fluor 514", preferredShortName:"AF514", excitMax:517, emissMax:542, casNo:"798557-07-0").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Alexa Fluor 532", preferredShortName:"AF532", excitMax:532, emissMax:553, casNo:"222159-92-4").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Alexa Fluor 546", preferredShortName:"AF546", excitMax:556, emissMax:573, casNo:"247145-23-9").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Alexa Fluor 555", preferredShortName:"AF555", excitMax:555, emissMax:565, casNo:"644990-77-2").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Alexa Fluor 568", preferredShortName:"AF568", excitMax:578, emissMax:603, casNo:"247145-38-6").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Alexa Fluor 594", preferredShortName:"AF594", excitMax:590, emissMax:617, casNo:"247145-86-4").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Alexa Fluor 610", preferredShortName:"AF610", excitMax:612, emissMax:628, casNo:"900528-62-3").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Alexa Fluor 633", preferredShortName:"AF633", excitMax:632, emissMax:647, casNo:"477780-06-6").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Alexa Fluor 635", preferredShortName:"AF635", excitMax:633, emissMax:647, casNo:"945850-82-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Alexa Fluor 647", preferredShortName:"AF647", excitMax:650, emissMax:665, casNo:"400051-23-2").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Alexa Fluor 660", preferredShortName:"AF660", excitMax:663, emissMax:690, casNo:"422309-89-5").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Alexa Fluor 680", preferredShortName:"AF680", excitMax:679, emissMax:702, casNo:"422309-67-9").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Alexa Fluor 700", preferredShortName:"AF700", excitMax:702, emissMax:723, casNo:"697795-05-4").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Alexa Fluor 750", preferredShortName:"AF750", excitMax:749, emissMax:775, casNo:"697795-06-5").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Alexa Fluor 790",             preferredShortName:"AF790",  excitMax:784, emissMax:814, casNo:"950891-33-5").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Allophycocyanin",             preferredShortName:"APC",    excitMax:650, emissMax:660, casNo:"-"          ).save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Anemonia Majano Cyan",        preferredShortName:"AmCyan", excitMax:457, emissMax:489, casNo:"1216872-44-4").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Aminomethylcoumarin Acetate", preferredShortName:"AMCA",   excitMax:346, emissMax:448, casNo:"106562-32-7").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Allophycocyanin-Alexa Fluor 680", preferredShortName:"APC-AF680", excitMax:655, emissMax:704, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Allophycocyanin-Alexa Fluor 700", preferredShortName:"APC-AF700", excitMax:655, emissMax:718, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Allophycocyanin-Alexa Fluor 750", preferredShortName:"APC-AF750", excitMax:650, emissMax:775, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Allophycocyanin-Cy5.5", preferredShortName:"APC-Cy5.5", excitMax:650, emissMax:695, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Allophycocyanin-Cy7", preferredShortName:"APC-Cy7", excitMax:650, emissMax:767, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Allophycocyanin-eFluor 750", preferredShortName:"APC-eFluor750", excitMax:650, emissMax:750, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Allophycocyanin-eFluor 780", preferredShortName:"APC-eFluor780", excitMax:650, emissMax:780, casNo:"1472056-77-1").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Allophycocyanin-H7", preferredShortName:"APC-H7", excitMax:650, emissMax:765, casNo:"1366000-62-5").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Allophycocyanin-Vio770", preferredShortName:"APC-Vio770", excitMax:652, emissMax:775, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Atto488", preferredShortName:"ATTO488", excitMax:501, emissMax:523, casNo:"923585-42-6").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"BODIPY FL", preferredShortName:"BODIPY-FL", excitMax:502, emissMax:511, casNo:"146616-66-2*").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"BODIPY R6G", preferredShortName:"BODIPY-R6G", excitMax:527, emissMax:547, casNo:"335193-70-9").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Brilliant Blue 515", preferredShortName:"BB515", excitMax:490, emissMax:515, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Brilliant Utraviolet 395", preferredShortName:"BUV395", excitMax:348, emissMax:395, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Brilliant Utraviolet 496", preferredShortName:"BUV496", excitMax:348, emissMax:496, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Brilliant Utraviolet 563", preferredShortName:"BUV563", excitMax:348, emissMax:563, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Brilliant Utraviolet 661", preferredShortName:"BUV661", excitMax:348, emissMax:661, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Brilliant Utraviolet 737", preferredShortName:"BUV737", excitMax:348, emissMax:737, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Brilliant Utraviolet 805", preferredShortName:"BUV805", excitMax:348, emissMax:805, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Brilliant Violet 421", preferredShortName:"BV421", excitMax:406, emissMax:423, casNo:"1428441-68-2").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Brilliant Violet 480", preferredShortName:"BV480", excitMax:436, emissMax:478, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Brilliant Violet 510", preferredShortName:"BV510", excitMax:405, emissMax:510, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Brilliant Violet 570", preferredShortName:"BV570", excitMax:407, emissMax:571, casNo:"1428441-76-2").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Brilliant Violet 605", preferredShortName:"BV605", excitMax:407, emissMax:603, casNo:"1632128-60-9").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Brilliant Violet 650", preferredShortName:"BV650", excitMax:407, emissMax:647, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Brilliant Violet 711", preferredShortName:"BV711", excitMax:405, emissMax:711, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Brilliant Violet 785", preferredShortName:"BV785", excitMax:405, emissMax:786, casNo:"1613592-44-1").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Brilliant Violet 786", preferredShortName:"BV786", excitMax:407, emissMax:786, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Calcein", preferredShortName:"Calcein", excitMax:493, emissMax:514, casNo:"1461-15-0").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Calcein Blue AM", preferredShortName:"Calcein-BAM", excitMax:360, emissMax:445, casNo:"168482-84-6").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Calcein Violet AM", preferredShortName:"Calcein-VAM", excitMax:400, emissMax:452, casNo:"148504-34-1").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Calcium Sensor Dye eFluor 514", preferredShortName:"CSD-eFluor514", excitMax:490, emissMax:514, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Carboxyfluorescein Diacetate Succinimidyl Ester", preferredShortName:"CFDA-SE", excitMax:492, emissMax:517, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Carboxyfluorescein Succinimidyl Ester", preferredShortName:"CFSE", excitMax:492, emissMax:525, casNo:"150347-59-4").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Cascade Blue", preferredShortName:"Cascade-B", excitMax:401, emissMax:420, casNo:"1325-87-7").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Cascade Yellow", preferredShortName:"Cascade-Y", excitMax:400, emissMax:550, casNo:"220930-95-0").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Cell Proliferation Dye eFluor 450", preferredShortName:"CPD-eFluor450", excitMax:405, emissMax:445, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Cell Proliferation Dye eFluor 670", preferredShortName:"CPD-eFluor670", excitMax:652, emissMax:672, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"CellTrace Violet Cell Proliferation", preferredShortName:"CellTrace-Vio", excitMax:392, emissMax:455, casNo:"1437217-10-1").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"CellVue Claret", preferredShortName:"CellVue-Claret", excitMax:655, emissMax:657, casNo:"1042142-46-0").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Cyanine 2", preferredShortName:"Cy2", excitMax:492, emissMax:507, casNo:"102185-03-5").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Cyanine 3", preferredShortName:"Cy3", excitMax:552, emissMax:566, casNo:"146368-16-3").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Cyanine 3.5", preferredShortName:"Cy3.5", excitMax:581, emissMax:598, casNo:"189767-45-1").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Cyanine 5", preferredShortName:"Cy5", excitMax:633, emissMax:670, casNo:"144377-05-9").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Cyanine 5.5", preferredShortName:"Cy5.5", excitMax:677, emissMax:695, casNo:"210892-23-2").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Cyanine 7", preferredShortName:"Cy7", excitMax:743, emissMax:767, casNo:"169799-14-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"CyQUANT DNA", preferredShortName:"CyQUANT-GR", excitMax:502, emissMax:522, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"CyTRAK Orange", preferredShortName:"CyTRAK-O", excitMax:514, emissMax:609, casNo:"1195771-25-5").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"DAPI", preferredShortName:"DAPI", excitMax:358, emissMax:462, casNo:"47165-04-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"DiA", preferredShortName:"DiA", excitMax:455, emissMax:586, casNo:"371114-38-4").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dichloro-Dihydro-Fluorescein", preferredShortName:"DCF", excitMax:505, emissMax:525, casNo:"106070-31-9").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"DiD", preferredShortName:"DiD", excitMax:647, emissMax:669, casNo:"127274-91-3").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dil", preferredShortName:"Dil", excitMax:550, emissMax:568, casNo:"41085-99-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"DiO", preferredShortName:"DiO", excitMax:489, emissMax:506, casNo:"34215-57-1").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"DiR", preferredShortName:"DiR", excitMax:750, emissMax:781, casNo:"100068-60-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"DRAQ5", preferredShortName:"DRAQ5", excitMax:645, emissMax:683, casNo:"254098-36-7").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"DRAQ7", preferredShortName:"DRAQ7", excitMax:599, emissMax:694, casNo:"1533453-55-2").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Discosoma sp. Red Fluorescent Protein", preferredShortName:"DsRED", excitMax:532, emissMax:595, casNo:"469863-23-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"DyLight 350", preferredShortName:"DL350", excitMax:353, emissMax:432, casNo:"1436849-83-0").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"DyLight 405", preferredShortName:"DL405", excitMax:400, emissMax:420, casNo:"1051927-09-3").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"DyLight 488", preferredShortName:"DL488", excitMax:493, emissMax:518, casNo:"1051927-12-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"DyLight 549", preferredShortName:"DL549", excitMax:562, emissMax:576, casNo:"1051927-13-9").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"DyLight 550", preferredShortName:"DL550", excitMax:562, emissMax:576, casNo:"1340586-78-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"DyLight 594", preferredShortName:"DL594", excitMax:593, emissMax:618, casNo:"1268612-00-5").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"DyLight 633", preferredShortName:"DL633", excitMax:638, emissMax:658, casNo:"1051927-14-0").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"DyLight 649", preferredShortName:"DL649", excitMax:654, emissMax:670, casNo:"1051927-15-1").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"DyLight 650", preferredShortName:"DL650", excitMax:652, emissMax:672, casNo:"1364214-13-0").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"DyLight 800", preferredShortName:"DL800", excitMax:777, emissMax:794, casNo:"1051927-23-1").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"DyLight 680", preferredShortName:"DL680", excitMax:682, emissMax:712, casNo:"1051927-24-2").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-350", preferredShortName:"DY-350", excitMax:353, emissMax:432, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-405", preferredShortName:"DY-405", excitMax:400, emissMax:423, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-415", preferredShortName:"DY-415", excitMax:418, emissMax:467, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-430", preferredShortName:"DY-430", excitMax:444, emissMax:487, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-431", preferredShortName:"DY-431", excitMax:442, emissMax:496, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-478", preferredShortName:"DY-478", excitMax:478, emissMax:518, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-490", preferredShortName:"DY-490", excitMax:491, emissMax:515, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-495", preferredShortName:"DY-495", excitMax:493, emissMax:521, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-505", preferredShortName:"DY-505", excitMax:505, emissMax:530, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-530", preferredShortName:"DY-530", excitMax:539, emissMax:561, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-548", preferredShortName:"DY-548", excitMax:558, emissMax:572, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-547", preferredShortName:"DY-547", excitMax:551, emissMax:565, casNo:"947138-67-2").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-549", preferredShortName:"DY-549", excitMax:550, emissMax:575, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-550", preferredShortName:"DY-550", excitMax:553, emissMax:578, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-554", preferredShortName:"DY-554", excitMax:551, emissMax:572, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-555", preferredShortName:"DY-555", excitMax:547, emissMax:572, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-556", preferredShortName:"DY-556", excitMax:548, emissMax:573, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-560", preferredShortName:"DY-560", excitMax:559, emissMax:578, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-590", preferredShortName:"DY-590", excitMax:580, emissMax:599, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-591", preferredShortName:"DY-591", excitMax:581, emissMax:598, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-605", preferredShortName:"DY-605", excitMax:600, emissMax:624, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-594", preferredShortName:"DY-594", excitMax:594, emissMax:615, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-610", preferredShortName:"DY-610", excitMax:610, emissMax:630, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-615", preferredShortName:"DY-615", excitMax:621, emissMax:641, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-630", preferredShortName:"DY-630", excitMax:636, emissMax:657, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-631", preferredShortName:"DY-631", excitMax:637, emissMax:658, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-632", preferredShortName:"DY-632", excitMax:637, emissMax:657, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-633", preferredShortName:"DY-633", excitMax:637, emissMax:657, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-634", preferredShortName:"DY-634", excitMax:635, emissMax:658, casNo:"1189010-49-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-635", preferredShortName:"DY-635", excitMax:647, emissMax:671, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-636", preferredShortName:"DY-636", excitMax:645, emissMax:671, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-648", preferredShortName:"DY-648", excitMax:653, emissMax:674, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-647", preferredShortName:"DY-647", excitMax:653, emissMax:672, casNo:"890317-39-2").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-650", preferredShortName:"DY-650", excitMax:653, emissMax:674, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-649", preferredShortName:"DY-649", excitMax:655, emissMax:676, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-651", preferredShortName:"DY-651", excitMax:656, emissMax:678, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-652", preferredShortName:"DY-652", excitMax:654, emissMax:675, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-654", preferredShortName:"DY-654", excitMax:653, emissMax:677, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-675", preferredShortName:"DY-675", excitMax:674, emissMax:699, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-676", preferredShortName:"DY-676", excitMax:674, emissMax:699, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-677", preferredShortName:"DY-677", excitMax:673, emissMax:694, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-678", preferredShortName:"DY-678", excitMax:674, emissMax:694, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-679", preferredShortName:"DY-679", excitMax:679, emissMax:697, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-680", preferredShortName:"DY-680", excitMax:690, emissMax:709, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-681", preferredShortName:"DY-681", excitMax:691, emissMax:708, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-682", preferredShortName:"DY-682", excitMax:692, emissMax:709, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-700", preferredShortName:"DY-700", excitMax:707, emissMax:730, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-701", preferredShortName:"DY-701", excitMax:706, emissMax:731, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-703", preferredShortName:"DY-703", excitMax:705, emissMax:721, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-704", preferredShortName:"DY-704", excitMax:706, emissMax:721, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-730", preferredShortName:"DY-730", excitMax:732, emissMax:758, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-731", preferredShortName:"DY-731", excitMax:736, emissMax:760, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-732", preferredShortName:"DY-732", excitMax:736, emissMax:759, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-734", preferredShortName:"DY-734", excitMax:736, emissMax:759, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-749", preferredShortName:"DY-749", excitMax:759, emissMax:780, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-750", preferredShortName:"DY-750", excitMax:747, emissMax:776, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-751", preferredShortName:"DY-751", excitMax:751, emissMax:779, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-752", preferredShortName:"DY-752", excitMax:748, emissMax:772, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-754", preferredShortName:"DY-754", excitMax:748, emissMax:771, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-776", preferredShortName:"DY-776", excitMax:771, emissMax:793, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-777", preferredShortName:"DY-777", excitMax:770, emissMax:788, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-778", preferredShortName:"DY-778", excitMax:767, emissMax:787, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-780", preferredShortName:"DY-780", excitMax:782, emissMax:800, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-781", preferredShortName:"DY-781", excitMax:783, emissMax:800, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-782", preferredShortName:"DY-782", excitMax:783, emissMax:800, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-800", preferredShortName:"DY-800", excitMax:777, emissMax:791, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"EdU Alexa Fluor 488", preferredShortName:"EdU-AF488", excitMax:496, emissMax:516, casNo:"1431149-10-8").save())
            // achtung!!!!
//                        probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-831", preferredShortName:"DY-831", excitMax:844, emissMax:"-", casNo:"-").save()) // emissMax no Integer!!!!
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Dynomics DY-831", preferredShortName:"DY-831", excitMax:844, emissMax:0, casNo:"-").save()) // emissMax no Integer!!!!
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"EdU Alexa Fluor 647", preferredShortName:"EdU-AF647", excitMax:650, emissMax:665, casNo:"1431155-07-5").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"EdU Pacific Blue", preferredShortName:"EdU-PacBlue", excitMax:405, emissMax:455, casNo:"1431148-70-7").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"eFluor 450", preferredShortName:"eFluor450", excitMax:400, emissMax:450, casNo:"1592653-87-6").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"eFluor 490", preferredShortName:"eFluor490", excitMax:350, emissMax:490, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"eFluor 506 Fixable Viability Dye", preferredShortName:"eFluor506-FVD", excitMax:420, emissMax:506, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"eFluor 525", preferredShortName:"eFluor525", excitMax:350, emissMax:525, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"eFluor 565", preferredShortName:"eFluor565", excitMax:350, emissMax:565, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"eFluor 585", preferredShortName:"eFluor585", excitMax:350, emissMax:604, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"eFluor 605", preferredShortName:"eFluor605", excitMax:350, emissMax:605, casNo:"1248429-27-7").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"eFluor 615", preferredShortName:"eFluor615", excitMax:590, emissMax:622, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"eFluor 625", preferredShortName:"eFluor625", excitMax:350, emissMax:625, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"eFluor 650", preferredShortName:"eFluor650", excitMax:350, emissMax:650, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"eFluor 670", preferredShortName:"eFluor670", excitMax:647, emissMax:670, casNo:"1437243-07-6").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"eFluor 660", preferredShortName:"eFluor660", excitMax:633, emissMax:658, casNo:"1634649-16-3").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"eFluor 700", preferredShortName:"eFluor700", excitMax:350, emissMax:700, casNo:"1431525-04-0").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"eFluor 710", preferredShortName:"eFluor710", excitMax:350, emissMax:710, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"eFluor 780 Fixable Viability Dye", preferredShortName:"eFluor780-FVD", excitMax:755, emissMax:780, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Emerald 300", preferredShortName:"Emerald300", excitMax:289, emissMax:530, casNo:"608142-29-6").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Enhanced Cyan Flourescent Protein", preferredShortName:"ECFP", excitMax:435, emissMax:477, casNo:"1416379-03-7").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Enhanced Green Fluorescent Protein", preferredShortName:"EGFP", excitMax:480, emissMax:510, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Enhanced Yellow Fluorescent Protein", preferredShortName:"EYFP", excitMax:515, emissMax:528, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Eosin", preferredShortName:"Eosin", excitMax:525, emissMax:546, casNo:"17372-87-1").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Ethidium Bromide", preferredShortName:"EtBr", excitMax:523, emissMax:604, casNo:"1239-45-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Ethidium Homodimer-1", preferredShortName:"EtHD-1", excitMax:528, emissMax:617, casNo:"61926-22-5").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Ethidium Monoazide EMA", preferredShortName:"Et-MonoAz-EMA", excitMax:510, emissMax:590, casNo:"58880-05-0").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Fluo-3", preferredShortName:"Fluo-3", excitMax:506, emissMax:526, casNo:"123632-39-3").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Fluo-4", preferredShortName:"Fluo-4", excitMax:494, emissMax:516, casNo:"273221-59-3").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Fluorescein Isothiocyanate", preferredShortName:"FITC", excitMax:500, emissMax:520, casNo:"27072-45-3").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Fluorescein Mercuric Acetate", preferredShortName:"FLMA", excitMax:495, emissMax:520, casNo:"75350-46-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Fluoro-Emerald", preferredShortName:"Fluoro-Emerald", excitMax:495, emissMax:523, casNo:"194369-11-4").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Fura Red", preferredShortName:"Fura-Red", excitMax:435, emissMax:670, casNo:"149732-62-7").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Fura-3", preferredShortName:"Fura3", excitMax:325, emissMax:510, casNo:"172890-84-5").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"FxCycle Far Red", preferredShortName:"FxCycle-FR", excitMax:640, emissMax:658, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"FxCycle Violet", preferredShortName:"FxCycle-Vio", excitMax:358, emissMax:462, casNo:"28718-90-3").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Green Fluorescent Protein - Violet Excitation", preferredShortName:"GFP-Vex", excitMax:398, emissMax:515, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Green Flourescent Protein", preferredShortName:"GFP", excitMax:488, emissMax:515, casNo:"147336-22-9").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"HiLyte Fluor 488", preferredShortName:"HLF488", excitMax:501, emissMax:527, casNo:"1051927-29-7").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"HiLyte Fluor 555", preferredShortName:"HLF555", excitMax:550, emissMax:566, casNo:"1051927-30-0").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"HiLyte Fluor 647", preferredShortName:"HLF647", excitMax:649, emissMax:674, casNo:"925693-87-4").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"HiLyte Fluor 680", preferredShortName:"HLF680", excitMax:678, emissMax:699, casNo:"1051927-34-4").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"HiLyte Fluor 750", preferredShortName:"HLF750", excitMax:754, emissMax:778, casNo:"1051927-32-2").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Hoechst 33258", preferredShortName:"Hoechst-33258", excitMax:345, emissMax:455, casNo:"23491-45-4").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Hoechst 33342", preferredShortName:"Hoechst-33342", excitMax:343, emissMax:455, casNo:"23491-52-3").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Hydroxycoumarin", preferredShortName:"Hydroxycoumarin", excitMax:360, emissMax:450, casNo:"43070-85-5").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Indo-1 Calcium Indicator", preferredShortName:"Indo-1", excitMax:347, emissMax:480, casNo:"96314-96-4").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"JC-1 Mitochondrial Potential Sensor", preferredShortName:"JC-1", excitMax:593, emissMax:595, casNo:"3520-43-2").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Krome Orange", preferredShortName:"Krome-O", excitMax:398, emissMax:530, casNo:"1558035-65-6").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Leadmium Green AM", preferredShortName:"Leadmium", excitMax:490, emissMax:520, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"LIVE/DEAD Fixable Aqua Dead Cell Stain", preferredShortName:"LIVE/DEAD-Aqua", excitMax:367, emissMax:526, casNo:"1431148-98-9").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"LIVE/DEAD Fixable Blue Dead Cell Stain", preferredShortName:"LIVE/DEAD-Blue", excitMax:343, emissMax:442, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"LIVE/DEAD Fixable Far Red Dead Cell Stain", preferredShortName:"LIVE/DEAD-FarRed", excitMax:650, emissMax:670, casNo:"1431155-49-5").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"LIVE/DEAD Fixable Green Dead Cell Stain", preferredShortName:"LIVE/DEAD-Green", excitMax:498, emissMax:525, casNo:"1431152-82-7").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"LIVE/DEAD Fixable Near-IR Dead Cell Stain", preferredShortName:"LIVE/DEAD-Near-IR", excitMax:752, emissMax:776, casNo:"1431159-71-5").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"LIVE/DEAD Fixable Red Dead Cell Stain", preferredShortName:"LIVE/DEAD-Red", excitMax:594, emissMax:612, casNo:"1431152-85-0").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"LIVE/DEAD Fixable Violet Dead Cell Stain", preferredShortName:"LIVE/DEADiViolet", excitMax:403, emissMax:455, casNo:"1431148-96-7").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"LIVE/DEAD Fixable Yellow Dead Cell Stain", preferredShortName:"LIVE/DEAD-Yellow", excitMax:401, emissMax:551, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Lucifer Yellow", preferredShortName:"Lucifer-Yellow", excitMax:428, emissMax:544, casNo:"82446-52-4").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Magnesium Green", preferredShortName:"Magnesium-Green", excitMax:507, emissMax:531, casNo:"170516-41-3").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Marina Blue", preferredShortName:"Marina-Blue", excitMax:364, emissMax:461, casNo:"215868-23-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"mBanana", preferredShortName:"mBanana", excitMax:540, emissMax:553, casNo:"1114839-40-5").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"mCherry", preferredShortName:"mCherry", excitMax:587, emissMax:610, casNo:"1628764-31-7").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"mCitrine", preferredShortName:"mCitrine", excitMax:516, emissMax:529, casNo:"1357606-54-2").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"MethylCoumarin", preferredShortName:"MetCoumarin", excitMax:360, emissMax:448, casNo:"1333-47-7").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"MitoTracker Green", preferredShortName:"MitoTracker-Green", excitMax:490, emissMax:512, casNo:"1304563-13-0").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"MitoTracker Orange", preferredShortName:"MitoTracker-Orange", excitMax:550, emissMax:575, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"MitoTracker Red", preferredShortName:"MitoTracker-Red", excitMax:578, emissMax:598, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"mOrange", preferredShortName:"mOrange", excitMax:548, emissMax:562, casNo:"1114839-60-9").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"mPlum", preferredShortName:"mPlum", excitMax:590, emissMax:649, casNo:"1399820-93-9").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"mRaspberry", preferredShortName:"mRaspberry", excitMax:597, emissMax:624, casNo:"1452799-41-5").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"mRed Fluorescent Protein", preferredShortName:"mRFP", excitMax:584, emissMax:607, casNo:"1452799-30-2").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"mStrawberry", preferredShortName:"mStrawberry", excitMax:574, emissMax:596, casNo:"1114834-99-9").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Na-Green", preferredShortName:"Na-Green", excitMax:506, emissMax:532, casNo:"195244-55-4").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Nile Red", preferredShortName:"Nile-Red", excitMax:559, emissMax:637, casNo:"7385-67-3").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Ou2010Cresolphthalein Complexone", preferredShortName:"CPC", excitMax:488, emissMax:660, casNo:"2411-89-4").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Oregon Green", preferredShortName:"Oregon-Gr488", excitMax:491, emissMax:519, casNo:"195136-58-4").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Oregon Green 514", preferredShortName:"Oregon-Gr514", excitMax:510, emissMax:532, casNo:"198139-53-6").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Pacific Blue", preferredShortName:"PacBlue", excitMax:405, emissMax:455, casNo:"215868-31-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Pacific Orange", preferredShortName:"PacOrange", excitMax:403, emissMax:551, casNo:"1122414-42-9").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Phycoerythrin-Alexa Fluor 610", preferredShortName:"PE-AF610", excitMax:563, emissMax:628, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Phycoerythrin-Alexa Fluor 647", preferredShortName:"PE-AF647", excitMax:567, emissMax:669, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Phycoerythrin-Alexa Fluor 680", preferredShortName:"PE-AF680", excitMax:570, emissMax:702, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Phycoerythrin-Alexa Fluor 700", preferredShortName:"PE-AF700", excitMax:563, emissMax:720, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Phycoerythrin-Alexa Fluor 750", preferredShortName:"PE-AF750", excitMax:570, emissMax:776, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Phycoerythrin-CF594", preferredShortName:"PE-CF594", excitMax:564, emissMax:612, casNo:"1613592-67-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Phycoerythrin-Cy5", preferredShortName:"PE-Cy5", excitMax:565, emissMax:667, casNo:"1448849-77-1").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Phycoerythrin-Cy5.5", preferredShortName:"PE-Cy5.5", excitMax:563, emissMax:695, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Phycoerythrin-Cy7", preferredShortName:"PE-Cy7", excitMax:563, emissMax:760, casNo:"1429496-42-3").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Phycoerythrin-DY590", preferredShortName:"PE-DY590", excitMax:563, emissMax:599, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Phycoerythrin-DY647", preferredShortName:"PE-DY647", excitMax:563, emissMax:672, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Peridinin-Chlorophyll Proteins", preferredShortName:"PerCP", excitMax:490, emissMax:675, casNo:"422551-33-5").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Peridinin-Chlorophyll Proteins-Cy5.5", preferredShortName:"PerCP-Cy5.5", excitMax:488, emissMax:695, casNo:"1474026-81-7").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Peridinin-Chlorophyll Proteins-eFluor 710", preferredShortName:"PerCP-eFluor710", excitMax:488, emissMax:710, casNo:"1353683-31-4").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Phycoerythrin-Texas Red", preferredShortName:"PE-TxRed", excitMax:563, emissMax:613, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Phycoerythrin-Vio770", preferredShortName:"PE-Vio770", excitMax:565, emissMax:775, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"pHrodo Green STP Ester ", preferredShortName:"pHrodo-Green", excitMax:509, emissMax:533, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"pHrodo Red, succinimidyl ester", preferredShortName:"pHrodo-Red", excitMax:560, emissMax:585, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Phycocyanin", preferredShortName:"PC", excitMax:617, emissMax:646, casNo:"11016-15-2").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Phycoerythrin", preferredShortName:"PE", excitMax:563, emissMax:578, casNo:"11016-17-4").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"PicoGreen", preferredShortName:"PicoGreen", excitMax:502, emissMax:522, casNo:"177571-06-1").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"PKH2", preferredShortName:"PKH2-Green", excitMax:490, emissMax:504, casNo:"145687-07-6").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"PKH26", preferredShortName:"PKH26-Red ", excitMax:551, emissMax:567, casNo:"154214-55-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"PKH67", preferredShortName:"PKH67", excitMax:490, emissMax:504, casNo:"257277-27-3").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"POPO-1 Iodide", preferredShortName:"POPO-1", excitMax:433, emissMax:457, casNo:"169454-15-3").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"PO-PRO-1 Iodide", preferredShortName:"PO-PRO-1", excitMax:435, emissMax:457, casNo:"157199-56-9").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Propidium Iodide", preferredShortName:"PI", excitMax:350, emissMax:617, casNo:"25535-16-4").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Pyronin Y", preferredShortName:"Pyronin-Y", excitMax:547, emissMax:560, casNo:"92-32-0").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Quantum Dots 525", preferredShortName:"QD525", excitMax:350, emissMax:525, casNo:"885332-45-6").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Quantum Dots 545", preferredShortName:"QD545", excitMax:350, emissMax:545, casNo:"948906-89-6").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Quantum Dots 565", preferredShortName:"QD565", excitMax:350, emissMax:565, casNo:"859509-02-7").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Quantum Dots 585", preferredShortName:"QD585", excitMax:350, emissMax:585, casNo:"885332-46-7").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Quantum Dots 605", preferredShortName:"QD605", excitMax:350, emissMax:605, casNo:"849813-89-4").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Quantum Dots 625", preferredShortName:"QD625", excitMax:350, emissMax:625, casNo:"1144512-19-5").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Quantum Dots 655", preferredShortName:"QD655", excitMax:350, emissMax:655, casNo:"674287-64-0").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Quantum Dots 705", preferredShortName:"QD705", excitMax:350, emissMax:705, casNo:"885332-47-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Quantum Dots 800", preferredShortName:"QD800", excitMax:350, emissMax:800, casNo:"885332-50-3").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Rhodamine", preferredShortName:"Rho", excitMax:550, emissMax:570, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Rhodamine 110", preferredShortName:"Rho110", excitMax:497, emissMax:520, casNo:"13558-31-1").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Rhodamine 123", preferredShortName:"Rho123", excitMax:507, emissMax:529, casNo:"62669-70-9").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Rhodamine Green", preferredShortName:"RhoGreen", excitMax:505, emissMax:527, casNo:"189200-71-3").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Rhodamine Red", preferredShortName:"RhoRed", excitMax:573, emissMax:591, casNo:"99752-92-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Rhodamine Red-X", preferredShortName:"RhoRed-X", excitMax:560, emissMax:580, casNo:"178623-12-6").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"RiboFlavin", preferredShortName:"RiboFlavin", excitMax:266, emissMax:531, casNo:"83-88-5").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"SNARF-1", preferredShortName:"SNARF1", excitMax:549, emissMax:586, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Spectral Red", preferredShortName:"Spectral-Red", excitMax:506, emissMax:665, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"SureLight P1", preferredShortName:"SureLight-P1", excitMax:545, emissMax:667, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"SureLight P3", preferredShortName:"SureLight-P3", excitMax:614, emissMax:662, casNo:"1365659-06-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"SYBR Green", preferredShortName:"SYBRGreen", excitMax:498, emissMax:522, casNo:"217087-73-5").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"SYTO 11", preferredShortName:"SYTO-11", excitMax:506, emissMax:526, casNo:"173080-67-6").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"SYTO 13", preferredShortName:"SYTO-13", excitMax:488, emissMax:506, casNo:"173080-69-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"SYTO 16", preferredShortName:"SYTO-16", excitMax:488, emissMax:520, casNo:"173080-72-3").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"SYTO 17", preferredShortName:"SYTO-17", excitMax:618, emissMax:637, casNo:"189233-66-7").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"SYTO 45", preferredShortName:"SYTO-45", excitMax:450, emissMax:486, casNo:"335078-86-9").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"SYTO 59", preferredShortName:"SYTO-59", excitMax:622, emissMax:643, casNo:"235422-34-1").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"SYTO 60", preferredShortName:"SYTO-60", excitMax:650, emissMax:681, casNo:"335079-14-6").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"SYTO 62", preferredShortName:"SYTO-62", excitMax:650, emissMax:681, casNo:"286951-08-4").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"SYTO 61", preferredShortName:"SYTO-61", excitMax:618, emissMax:651, casNo:"335079-15-7").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"SYTO 82", preferredShortName:"SYTO-82", excitMax:540, emissMax:560, casNo:"335079-10-2").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"SYTO 9", preferredShortName:"SYTO-9", excitMax:482, emissMax:500, casNo:"208540-89-0").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"SYTOX AADvanced Stain", preferredShortName:"SYTOX-AAD", excitMax:546, emissMax:646, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"SYTOX Blue Stain", preferredShortName:"SYTOX-B", excitMax:431, emissMax:480, casNo:"396077-00-2").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"SYTOX Green Stain", preferredShortName:"SYTOX-G", excitMax:504, emissMax:523, casNo:"194100-76-0").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"SYTOX Orange Stain", preferredShortName:"SYTOX-O", excitMax:547, emissMax:570, casNo:"324767-53-5").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"SYTOX Red Stain", preferredShortName:"SYTOX-R", excitMax:640, emissMax:658, casNo:"915152-67-9").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"tdTomato", preferredShortName:"tdTomato", excitMax:554, emissMax:581, casNo:"1114838-94-6").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Texas Red", preferredShortName:"TxRed", excitMax:589, emissMax:615, casNo:"82354-19-6").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Thiazole Orange", preferredShortName:"Thiazole-Orange", excitMax:500, emissMax:530, casNo:"107091-89-4").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"ThiolTracker Violet", preferredShortName:"ThiolTracker-Vio", excitMax:406, emissMax:526, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"TO-PRO-1", preferredShortName:"TO-PRO-1", excitMax:509, emissMax:533, casNo:"157199-59-2").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"TO-PRO-3", preferredShortName:"TO-PRO-3", excitMax:642, emissMax:661, casNo:"157199-63-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"TOTO-1", preferredShortName:"TOTO-1", excitMax:509, emissMax:533, casNo:"143413-84-7").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"TOTO-3", preferredShortName:"TOTO-3", excitMax:642, emissMax:661, casNo:"166196-17-4").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Tetramethylrhodamine", preferredShortName:"TRITC", excitMax:547, emissMax:572, casNo:"745735-42-6").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"TruRed", preferredShortName:"TruRed", excitMax:490, emissMax:695, casNo:"396076-95-2").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"V450", preferredShortName:"V450", excitMax:405, emissMax:448, casNo:"1257844-82-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"V500", preferredShortName:"V500", excitMax:415, emissMax:500, casNo:"1333160-12-5").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"VioBlue", preferredShortName:"VioBlue", excitMax:400, emissMax:452, casNo:"1431147-59-9").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"VioGreen", preferredShortName:"VioGreen", excitMax:388, emissMax:520, casNo:"-").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Vybrant DyeCycle Green", preferredShortName:"VDC-Green", excitMax:505, emissMax:535, casNo:"1431152-50-9").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Vybrant DyeCycle Orange", preferredShortName:"VDC-Orange", excitMax:518, emissMax:563, casNo:"1055990-89-0").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Vybrant DyeCycle Ruby", preferredShortName:"VDC-Ruby", excitMax:637, emissMax:686, casNo:"1345202-72-3").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Vybrant DyeCycle Violet", preferredShortName:"VDC-Violet", excitMax:370, emissMax:436, casNo:"1015439-88-9").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"Yellow Flourescent Protein", preferredShortName:"YFP", excitMax:505, emissMax:530, casNo:"1477601-52-7").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"YO-PRO-1", preferredShortName:"YO-PRO-1", excitMax:491, emissMax:506, casNo:"152068-09-2").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"YO-PRO-3", preferredShortName:"YO-PRO-3", excitMax:613, emissMax:629, casNo:"157199-62-7").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"YOYO-1", preferredShortName:"YOYO-1", excitMax:491, emissMax:509, casNo:"143413-85-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"YOYO-3", preferredShortName:"YOYO-3", excitMax:613, emissMax:629, casNo:"156312-20-8").save())
            probes.add(new Probe(probeTagDictionary: fluorescence, probeTag:"ZsGreen", preferredShortName:"ZsGreen", excitMax:494, emissMax:517, casNo:"1216871-88-3").save())



            probes.add(new Probe(probeTagDictionary: isotope, probeTag: "141 Praseodymium", preferredShortName:"141Pr" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"142 Neodymium",     preferredShortName:"142Nd" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"143 Neodymium",     preferredShortName:"143Nd" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"144 Neodymium",     preferredShortName:"144Nd" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"145 Neodymium",     preferredShortName:"145Nd" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"146 Neodymium",     preferredShortName:"146Nd" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"147 Samarium",      preferredShortName:"147Sm" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"148 Neodymium",     preferredShortName:"148Nd" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"149 Samarium",      preferredShortName:"149Sm" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"150 Neodymium",     preferredShortName:"150Nd" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"151 Europium",      preferredShortName:"151Eu" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"152 Samarium",      preferredShortName:"152Sm" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"153 Europium",      preferredShortName:"153Eu" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"154 Samarium",      preferredShortName:"154Sm" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"155 Gadolinium",    preferredShortName:"155Gd" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"156 Gadolinium",    preferredShortName:"156Gd" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"158 Gadolinium",    preferredShortName:"158Gd" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"159 Terbium",       preferredShortName:"159Tb" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"160 Gadolinium",    preferredShortName:"160Gd" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"161 Dysprosium",    preferredShortName:"161Dy" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"162 Dysprosium",    preferredShortName:"162Dy" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"163 Dysprosium",    preferredShortName:"163Dy" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"164 Dysprosium",    preferredShortName:"164Dy" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"165 Holmium",       preferredShortName:"165Ho" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"166 Erbium",        preferredShortName:"166Er" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"167 Erbium",        preferredShortName:"167Er" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"168 Erbium",        preferredShortName:"168Er" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"169 Thulium",       preferredShortName:"169Tm" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"170 Erbium",        preferredShortName:"170Er" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"171 Ytterbium",     preferredShortName:"171Yb" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"172 Ytterbium",     preferredShortName:"172Yb" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"173 Ytterbium",     preferredShortName:"173Yb" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"174 Ytterbium",     preferredShortName:"174Yb" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"175 Lutetium",      preferredShortName:"175Lu" ).save())
            probes.add(new Probe(probeTagDictionary: isotope, probeTag:"176 Ytterbium",     preferredShortName:"176Yb" ).save())



            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"3,3'-Diaminobenzidine",                                        preferredShortName:"DAB",            precColor:"Brown"       ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"3-amino-9-ethylcarbazole",                                     preferredShortName:"AEC",            precColor:"Red"         ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"5-bromo-4-chloro-3-indolyl phosphate  nitro blue tetrazolium", preferredShortName:"BCIP-NBT",       precColor:"Indigo"      ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"Fuchsin",                                                      preferredShortName:"Fuchsin",        precColor:"Red-Magenta" ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"Permanent Red",                                                preferredShortName:"PermRed",        precColor:"Magenta"     ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"Discovery Purple",                                             preferredShortName:"DiscPurple",     precColor:"Purple"      ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"Discovery Silver",                                             preferredShortName:"DiscSilver",     precColor:"Silver"      ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"Discovery Yellow",                                             preferredShortName:"DiscYellow",     precColor:"Yellow"      ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"3,3u2019,5,5u2019-tetramethylbenzidine",                       preferredShortName:"TMB",            precColor:"Blue"        ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"VECTOR Blue",                                                  preferredShortName:"vctrBlue",       precColor:"Blue"        ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"VECTOR Black",                                                 preferredShortName:"vctrBlack",      precColor:"Brown-Black" ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"ImmPACT AMEC",                                                 preferredShortName:"ipacAMEC",       precColor:"Red"         ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"VECTOR VIP",                                                   preferredShortName:"vctrVIP",        precColor:"Purple"      ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"VECTOR SG",                                                    preferredShortName:"vctrSG",         precColor:"Blue-Gray"   ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"VECTOR NovaRED",                                               preferredShortName:"vctrNovaRED",    precColor:"Red"         ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"5-bromo-4-chloro-3-indolyl-β-D-galactopyranoside",             preferredShortName:"X-gal",          precColor:"Blue-Green"  ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"Fast Red",                                                     preferredShortName:"FastRed",        precColor:"Magenta"     ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"Fast Blue",                                                    preferredShortName:"FastBlue",       precColor:"Blue"        ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"VECTOR Red",                                                   preferredShortName:"vctrRed",        precColor:"Magenta"     ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"Betazoid DAB",                                                 preferredShortName:"Betazoid-DAB",   precColor:"Brown"       ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"Cardassian DAB",                                               preferredShortName:"Cardassian-DAB", precColor:"Black Brown" ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"Romulin AEC",                                                  preferredShortName:"Romulin-AEC",    precColor:"Brick Red"   ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"Bajoran Purple",                                               preferredShortName:"Bajoran-Purple", precColor:"Purple"      ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"Vina Green",                                                   preferredShortName:"Vina-Green",     precColor:"Green"       ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"Deep Space Black",                                             preferredShortName:"DS-Black",       precColor:"Black"       ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"Warp Red",                                                     preferredShortName:"Warp-Red",       precColor:"Magenta"     ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"Vulcan Fast Red",                                              preferredShortName:"Vulcan-FastRed", precColor:"Magenta"     ).save())
            probes.add(new Probe(probeTagDictionary: chromogen, probeTag:"Ferangi Blue",                                                 preferredShortName:"Ferangi-Blue",   precColor:"Royal Blue"  ).save())

            probes
           /* */
    }

    def destroy = {
    }
}
