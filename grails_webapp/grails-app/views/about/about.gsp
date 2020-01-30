<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="layout" content="main">
  <title><g:message code="about.title.label" default="About"/></title>
</head>

<body>
<div class="col s12">
  <h2>About</h2>

  <p>To realize data-driven reproducible cytometry analysis and accelerate the adoption of computational analysis methods by both translational and clinical researchers,
  the US National Center for Advancing Translational Sciences (NCATS) funded a 5-year project in 2016 to develop a web-based computational infrastructure – FlowGate – to
  improve the accessibility and usability of the cutting-edge data analytical approaches to cytometry data processing and analysis. The main goals include creating an
  advanced user interface to support both bioinformaticians and also wet-lab translational and clinical investigators, making an analysis environment that is extensible
  to new methods and scalable to data size, and providing education and training for the appropriate use of the resulting data analysis pipelines.</p>

  <p><img class="img-fluid" style="height:500px;display: block;margin: auto;" src="${assetPath(src: 'flowgate_architecture.png')}"></p>

  <p>Design of the FlowGate architecture consists of a front-end user interface, a middle-tier management of data processing and analysis pipelines, and a back-end
  parallel computing environment. The back-end infrastructure is built upon a large cluster computer at San Diego Supercomputer Center - Comet, which has 1,944 compute
  nodes with 24 cores/28G DRAM/320G SSD memory on each node. Managed by the GenePattern (GP) platform, computational modules and pipelines on FlowGate can be easily
  expanded, with data/analysis provenance and software versioning. A GP Notebook environment, leveraging the Jupyter Notebook platform, is implemented in FlowGate to
  support interactive analytics, which is integrated with the front-end Java applications for customizable generation and download of data analysis results. Result
  visualization and statistical tables are included in a single HTML page for both on-line browsing and off-line sharing. Docker/Singularity containers are used to
  disseminate FlowGate software components for deployment on different computing platforms.</p>

  <p>Compared with other flow data analysis tools or platforms, the main features of FlowGate include:</p>

  <p><ul class="collection">
  <li class="collection-item">Extensible and scalable web-based analytics for each access to multiple computational methods and pipelines for reproducible data-driven analytics</li>
  <li class="collection-item">Advanced graphical user interface for visual analytics and interactive computing</li>
  <li class="collection-item">Machine learning classification for cell-based biomarker discovery</li>
</ul></p>
</div>

<div class="col s12">
  <p>The development team of FlowGate includes:</p>

  <div class="col s3">
    <ul class="collection with-header">
      <li class="collection-header"><h5>J. Craig Venter Institute</h5></li>
      <li class="collection-item">Yu “Max” Qian (PI)</li>
      <li class="collection-item">Richard H. Scheuermann (co-PI)</li>
      <li class="collection-item">Mehmet Kuscuoglu</li>
      <li class="collection-item">Ivan Chang</li>
      <li class="collection-item">Aishwarya Mandava</li>
    </ul>
  </div>

  <div class="col s3">
    <ul class="collection with-header">
      <li class="collection-header"><h5>Stanford University</h5></li>
      <li class="collection-header"><strong>Human Immune Monitoring Center & Department of Microbiology and Immunology:</strong></li>
      <li class="collection-item">Holden T. Maecker (site PI)</li>
      <li class="collection-item">Peter Acs</li>
    </ul>
  </div>

  <div class="col s3">
    <ul class="collection with-header">
      <li class="collection-header"><h5>University of California, San Diego</h5></li>
      <li class="collection-header"><strong>Department of Pathology:</strong></li>
      <li class="collection-item">Jack Bui (site PI)</li>
      <li class="collection-item">Huan-You Wang</li>
      <li class="collection-header"><strong>Department of Medicine:</strong></li>
      <li class="collection-item">Jill P. Mesirov (site PI)</li>
      <li class="collection-item">Michael Reich</li>
      <li class="collection-header"><strong>San Diego Supercomputer Center:</strong></li>
      <li class="collection-item">Robert Sinkovits (site PI)</li>
      <li class="collection-item">David Nadeau</li>
    </ul>
  </div>

  <div class="col s3">
    <ul class="collection with-header">
      <li class="collection-header"><h5>University of California, Irvine</h5></li>
      <li class="collection-header"><strong>Department of Computer Science:</strong></li>
      <li class="collection-item">Padhraic Smyth (site PI)</li>
      <li class="collection-item">Disi Ji</li>
      <li class="collection-item">Preston Putzel</li>
      <li class="collection-item">Eric Nalisnick</li>
      <li class="collection-header"><strong>Department of Pediatrics:</strong></li>
      <li class="collection-item">Dan M. Cooper (site PI)</li>
      <li class="collection-item">Frank Zaldivar</li>
      <li class="collection-item">Kim D. Lu</li>
    </ul>
  </div>
</div>

<div class="col s12">
  <p>External Advisory Board Members (alphabetical order):</p>

  <div class="card">
    <div class="card-content">
      <span class="card-title">Ryan Brinkman, Ph.D. – BC Cancer Agency</span>

      <p>Professor, Medical Genetics, Faculty of Medicine, UBC; Associate Faculty Member, Bioinformatics Centre, UBC; CEO, Cytapex Bioinformatics, Inc.
      Applying bioinformatics techniques (e.g., flowCore/flowUtils/flowClust) to flow cytometry data.  Also involved in the development of flow cytometry
      ontologies and minimum information standards.</p>
    </div>
  </div>

  <div class="card">
    <div class="card-content">
      <span class="card-title">Leonore Herzenberg, Ph.D. – Stanford</span>

      <p>Professor of Genetics; Chair of Flow Cytometry and Genetics.  Translational researcher interested in basic mechanisms that determine and regulate gene
      expression and cell function in the immune system. Developed new, user friendly and statistically reliable software to facilitate flow data analysis.</p>
    </div>
  </div>

  <div class="card">
    <div class="card-content">
      <span class="card-title">Brent L. Wood, M.D., Ph.D. – University of Washington</span>

      <p>Director of Hematopathology and Medical Director of SCCA pathology laboratories. He was responsible for implementing the first use of 9- and 10-color
      flow cytometry in the clinical laboratory, as well as subsequently exploiting its potential for the identification of minimal residual disease in acute
      lymphoid and myeloid leukemias.</p>
    </div>
  </div>

  <div class="card">
    <div class="card-content">
      <span class="card-title">Martin Zand, M.D., Ph.D. – University of Rochester Medical Center</span>

      <p>Professor of Medicine and Medical Humanities in the Division of Nephrology; Co-Director of the Clinical and Translational Science Institute; Director of
      the Rochester Center for Health Informatics. Research is focused on understanding how B cells respond to vaccines and organ transplants to produce antibodies,
      and understanding how to improve population health and healthcare delivery, using high dimensional clustering methods, graph theory, differential equation and
      stochastic branching process modeling.</p>
    </div>
  </div>
</div>

<div class="col s12">
  <blockquote>
    <b>Funding Agency:</b> National Institutes of Health/National Center for Advancing Translational Sciences<br/>
    <b>Grant Number:</b> U01TR001801<br/>
    <b>Funding Period:</b> September 2016 to June 2021<br/>
    <sec:ifLoggedIn>
      <b>Instructions:</b> <a target="_blank" href="${resource(dir: 'files', file: 'Instructions_using_flowgate_v1.pdf')}">Download</a>
    </sec:ifLoggedIn>
  </blockquote>
  <sec:ifLoggedIn>
    <p>Click <button class="btn"
                     onclick='window.location = "${createLink(controller: 'project', action: 'list')}"'>here</button> to create or view your projects.</p>
  </sec:ifLoggedIn>
</div>
</body>
</html>
