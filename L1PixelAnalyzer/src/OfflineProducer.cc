//#define DEBUG

#include "AnalysisExamples/L1PixelAnalyzer/interface/OfflineProducer.h"

#include "AnalysisExamples/AnalysisClasses/interface/SimpleJet.h"
#include "AnalysisExamples/AnalysisClasses/interface/Associator.h"
#include "AnalysisExamples/AnalysisClasses/interface/DeltaPhi.h"
#include "AnalysisExamples/AnalysisClasses/interface/L1PixelTrig.h"

// Classes to be stored
#include "AnalysisExamples/AnalysisObjects/interface/BaseJet.h"
#include "AnalysisExamples/AnalysisObjects/interface/BaseMEt.h"
#include "AnalysisExamples/AnalysisObjects/interface/OfflineMEt.h"
#include "AnalysisExamples/AnalysisObjects/interface/OfflineJet.h"
#include "AnalysisExamples/AnalysisObjects/interface/MCParticle.h"
#include "AnalysisExamples/AnalysisObjects/interface/SimplePixelJet.h"
#include "AnalysisExamples/AnalysisObjects/interface/GlobalMuon.h"
#include "AnalysisExamples/AnalysisObjects/interface/SimpleElectron.h"
#include "AnalysisExamples/AnalysisObjects/interface/SimpleTau.h"
#include "AnalysisExamples/AnalysisObjects/interface/Summary.h"

// For the offline jets and corrections
#include "DataFormats/JetReco/interface/CaloJetCollection.h"
#include "DataFormats/JetReco/interface/CaloJet.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
//#include "DataFormats/EgammaCandidates/interface/Electron.h"
//#include "DataFormats/EgammaCandidates/interface/ElectronFwd.h"
#include "DataFormats/EgammaCandidates/interface/PixelMatchGsfElectron.h"
#include "DataFormats/EgammaCandidates/interface/PixelMatchGsfElectronFwd.h"
#include "DataFormats/Candidate/interface/CandAssociation.h"
#include "DataFormats/METReco/interface/CaloMETCollection.h"
#include "JetMETCorrections/Objects/interface/JetCorrector.h"
#include "DataFormats/BTauReco/interface/IsolatedTauTagInfo.h"
// Invariant Mass from Calorimeter
#include "DataFormats/BTauReco/interface/TauMassTagInfo.h"

// For the b-tagging
#include "DataFormats/BTauReco/interface/JetTag.h"
//#include "DataFormats/JetReco/interface/JetTracksAssociation.h"
#include "DataFormats/BTauReco/interface/TrackIPTagInfo.h"

// For file output
#include <fstream>
#include <sstream>

#include <cmath>

#include <memory>

//
// constants, enums and typedefs
//

//
// static data member definitions
//

L1Trig OfflineProducer::L1Trigger;

//
// constructors and destructor
//
OfflineProducer::OfflineProducer(const edm::ParameterSet& iConfig) :
  conf_( iConfig ),
  CaloJetAlgorithm( iConfig.getUntrackedParameter<string>( "CaloJetAlgorithm" ) ),
  JetCorrectionService( iConfig.getUntrackedParameter<string>( "JetCorrectionService" ) ),
  METCollection( iConfig.getUntrackedParameter<string>( "METCollection" ) ),
  genParticleCandidates( iConfig.getUntrackedParameter<string>( "genParticleCandidates" ) ),
  trackCountingHighEffJetTags( iConfig.getUntrackedParameter<string>( "trackCountingHighEffJetTags" ) ),
  trackCountingHighPurJetTags( iConfig.getUntrackedParameter<string>( "trackCountingHighPurJetTags" ) ),
  impactParameterTagInfos( iConfig.getUntrackedParameter<string>( "impactParameterTagInfos" ) ),
  paramGlobalMuons_(iConfig.getUntrackedParameter<edm::InputTag>("ParamGlobalMuons") ),
  electronCandidates_(iConfig.getUntrackedParameter<edm::InputTag>("electronCandidates") ),
  electronHcalIsolation_(iConfig.getUntrackedParameter<edm::InputTag>("electronHcalIsolation") ),
  tauTagInfo_(iConfig.getUntrackedParameter<edm::InputTag>("tauTagInfo") ),
  numTkCut( iConfig.getUntrackedParameter<unsigned int>( "TracksMinimumNum_in_PixelJet" ) ),
  OutputEffFileName( iConfig.getUntrackedParameter<string>( "OutputEffFileName" ) ),
  cenJets_( iConfig.getParameter<string>( "CenJets" ) ),
  forJets_( iConfig.getParameter<string>( "ForJets" ) ),
  tauJets_( iConfig.getParameter<string>( "TauJets" ) ),
  l1MEt_( iConfig.getParameter<string>( "L1MEt" ) ),
  offlineJets_( iConfig.getParameter<string>( "OfflineJets" ) ),
  offlineMEt_( iConfig.getParameter<string>( "OfflineMEt" ) ),
  MCParticles_( iConfig.getParameter<string>( "MCParticles" ) ),
  simplePixelJets_( iConfig.getParameter<string>( "SimplePixelJets" ) ),
  globalMuons_( iConfig.getParameter<string>( "GlobalMuons" ) ),
  simpleElectrons_( iConfig.getParameter<string>( "SimpleElectrons" ) ),
  simpleTaus_( iConfig.getParameter<string>( "SimpleTaus" ) ),
  summary_( iConfig.getParameter<string>( "Summary" ) ),
  eventType_( iConfig.getParameter<unsigned int>( "EventType" ) )
{
  //now do what ever initialization is needed

//  OutputFile = new TFile((conf_.getUntrackedParameter<std::string>("OutputName")).c_str() ,"RECREATE","L1TrigPixelAnaOutput");
  // The file must be opened first, so that becomes the default position for all the histograms
//  OutputFile->cd();

  // White background for the canvases
  gROOT->SetStyle("Plain");

  HiVar = new HiVariables( ( conf_.getUntrackedParameter<string>("HiVarName") ).c_str() );

  eventcounter_ = 0;
//  PI_ = 3.141593;
  chargedpi_mass_ = 0.13957018;

  // L1
  produces<anaobj::BaseJetCollection>( cenJets_ );
  produces<anaobj::BaseJetCollection>( forJets_ );
  produces<anaobj::BaseJetCollection>( tauJets_ );
  produces<anaobj::BaseMEt>( l1MEt_ );
  // Offline
  produces<anaobj::OfflineJetCollection>( offlineJets_ );
  produces<anaobj::OfflineMEt>( offlineMEt_ );
  // MC
  produces<anaobj::MCParticleCollection>( MCParticles_ );
  // Simple pixel jets
  produces<anaobj::SimplePixelJetCollection>( simplePixelJets_ );
  // Global muons
  produces<anaobj::GlobalMuonCollection>( globalMuons_ );
  // SimpleElectrons
  produces<anaobj::SimpleElectronCollection>( simpleElectrons_ );
  // SimpleTaus
  produces<anaobj::SimpleTauCollection>( simpleTaus_ );
  // Summary
  produces<anaobj::Summary>( summary_ );
}


OfflineProducer::~OfflineProducer()
{
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)

  // Draw the histograms
  //  HiVar->Plot();

//  OutputFile->Write();
}

//
// member functions
//

// ------------ method called to for each event  ------------
void
OfflineProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;
  using namespace l1extra;
  using namespace std;
  using namespace anaobj;

  // L1 Calo
  edm::Handle < L1JetParticleCollection > l1eCenJets;
  edm::Handle < L1JetParticleCollection > l1eForJets;
  edm::Handle < L1JetParticleCollection > l1eTauJets;
  edm::Handle < L1EtMissParticle > l1eEtMiss;

  // Offline
  edm::Handle<reco::CaloMETCollection> caloMET;
  edm::Handle<reco::CaloJetCollection> caloJets;

  edm::Handle<reco::JetTagCollection> trackCountingHighEff;
  edm::Handle<reco::JetTagCollection> trackCountingHighPur;
  edm::Handle<reco::TrackIPTagInfoCollection> iPtagInfos;

  // Take genParticleCandidates
  edm::Handle< CandidateCollection > MCpartons;

  // ParamGlobalMuons
  edm::Handle< MuonCollection > paramGlobalMuons;

  // ElectronCandidates
  edm::Handle< PixelMatchGsfElectronCollection > electronCandidates;
  // Get the association vector
  edm::Handle< CandViewDoubleAssociations > electronHcalIsolation;

  // TauTagInfos
  edm::Handle<IsolatedTauTagInfoCollection> tauTagInfoHandle;


  // should get rid of this try/catch?
  try {
    edm::InputTag L1CJetLabel = conf_.getUntrackedParameter<edm::InputTag>("l1eCentralJetsSource");
    edm::InputTag L1FJetLabel = conf_.getUntrackedParameter<edm::InputTag>("l1eForwardJetsSource");
    edm::InputTag L1TauJetLabel = conf_.getUntrackedParameter<edm::InputTag>("l1eTauJetsSource");
    edm::InputTag L1EtMissLabel = conf_.getUntrackedParameter<edm::InputTag>("l1eEtMissSource");

    // Level 1
    iEvent.getByLabel(L1CJetLabel, l1eCenJets);
    iEvent.getByLabel(L1FJetLabel, l1eForJets);
    iEvent.getByLabel(L1TauJetLabel, l1eTauJets);
    iEvent.getByLabel(L1EtMissLabel, l1eEtMiss);
  }
  catch (...) {
    std::cerr << "L1TGCT: could not find one of the classes?" << std::endl;
    return;
  }

  try {
    // Offline MEt
    iEvent.getByLabel( METCollection, caloMET );
  }
  catch (...) {
    std::cerr << "Could not find METCollection" << std::endl;
    return;
  }
  try {
    // Offline IC5Jets
    iEvent.getByLabel( CaloJetAlgorithm, caloJets );
  }
  catch (...) {
    std::cerr << "Could not find IC5Jet collection" << std::endl;
    return;
  }
  try {
    // Btags
    iEvent.getByLabel( trackCountingHighEffJetTags, trackCountingHighEff );
    iEvent.getByLabel( trackCountingHighPurJetTags, trackCountingHighPur );
    iEvent.getByLabel( impactParameterTagInfos, iPtagInfos );
  }
  catch (...) {
    std::cerr << "Could not find the b-tagging collection" << std::endl;
      return;
  }
  try {
    // Take genParticleCandidates
    iEvent.getByLabel( genParticleCandidates, MCpartons );
  }
  catch (...) {
    std::cerr << "Could not find genParticleCandidates collection" << std::endl;
    return;
  }
  try {
    // Take the ParamGlobalMuons
    iEvent.getByLabel( paramGlobalMuons_, paramGlobalMuons );
  }
  catch (...) {
    std::cerr << "Could not find the ParamGlobalMuons collection" << std::endl;
  }
  try {
    // Take the ElectronCollection
    iEvent.getByLabel( electronCandidates_, electronCandidates );
    iEvent.getByLabel( electronHcalIsolation_, electronHcalIsolation );
  }
  catch (...) {
    std::cerr << "Could not find the ElectronCandidates colletion" << std::endl;
  }
  try {
    // Take the TauTagInfoCollection
    iEvent.getByLabel( tauTagInfo_, tauTagInfoHandle );
  }
  catch (...) {
    std::cerr << "Could not find the TauTagInfo collection" << std::endl;
  }


  eventcounter_++;
  if ( eventcounter_/100 == float(eventcounter_)/100. ) {
    std::cout << "Event number " << eventcounter_ << std::endl;
  }

  // ------- //
  // Offline //
  // ------- //

  // MEt
  // ---

  const reco::CaloMET * MET = &( *(caloMET->begin()) );

  // -----------------------------
  // ATTENTION
  // ---------
  // Not including DPhiMin for now
  // -----------------------------
  auto_ptr<OfflineMEt> offlineMEt( new OfflineMEt( MET->et(), MET->phi(), MET->sumEt(), MET->mEtSig(), 0. ) );

  iEvent.put( offlineMEt, offlineMEt_ );


  // Offline Jets IC5
  // ----------------

  auto_ptr<OfflineJetCollection> vec_IC5Jets_ptr( new OfflineJetCollection );

  int caloJetsSize = caloJets->size();

  // B-tagging
  // ---------
  // Brief description of the btagging algorithm (from the workbook page
  // https://twiki.cern.ch/twiki/bin/view/CMS/WorkBookBTagging):
  // -------------------------------------------------------------------
  // "Track Counting" algorithm: This is a very simple tag, exploiting the long lifetime of B hadrons.
  // It calculates the signed impact parameter significance of all good tracks, and orders them by decreasing significance.
  // It's b tag discriminator is defined as the significance of the N'th track, where a good value
  // is N = 2 (for high efficiency) or N = 3 (for high purity).
  // -------------------------------------------------------------------

  int tchenum = trackCountingHighEff->size();
  JetTagCollection::const_iterator bTagHighEff_it = trackCountingHighEff->begin();

  int tchpnum = trackCountingHighPur->size();
  JetTagCollection::const_iterator bTagHighPur_it = trackCountingHighPur->begin();

  int iptkinfonum = iPtagInfos->size();

  // Correct offline jets on the fly
  const JetCorrector* corrector = JetCorrector::getJetCorrector (JetCorrectionService, iSetup);

  int taginfocounter = 0;
  CaloJetCollection::const_iterator caloJets_it = caloJets->begin();
  TrackIPTagInfoCollection::const_iterator TkIpTagInfo_it = iPtagInfos->begin();
  for( ; TkIpTagInfo_it != iPtagInfos->end(); ++TkIpTagInfo_it ) {
    ++taginfocounter;
    double scale = corrector->correction( *caloJets_it );
    double corPt = scale*caloJets_it->pt();
    double corEt = scale*caloJets_it->et();

    int tkcount = 0;
    int tkNum = TkIpTagInfo_it->selectedTracks().size();
//     int probNum_0 = TkIpTagInfo_it->probabilities(0).size();
//     int probNum_1 = TkIpTagInfo_it->probabilities(1).size();

    double tkSumPt = 0.;
    const TrackRefVector & vec_TkColl = TkIpTagInfo_it->selectedTracks();
    RefVector<TrackCollection>::const_iterator TkColl_it = vec_TkColl.begin();
    for ( ; TkColl_it != vec_TkColl.end(); ++TkColl_it ) {
      tkSumPt += (*TkColl_it)->pt();
      ++tkcount;
    }

    // Jet mass
    double jetMass = 0;
    if ( corEt > corPt ) jetMass = sqrt( corEt*corEt - corPt*corPt );

    // Evaluate tag tracks invariant mass
    double bTagTkInvMass = tagTracksInvariantMass( vec_TkColl );

    // Fill the offline jets collection
    // --------------------------------

//     // Take the em and had energies to evaluate the emfraction
//     // See CMSSW/RecoJets/JetAlgorithms/src/JetMaker.cc.
//     double EmEnergyInEB = caloJets_it->emEnergyInEB();
//     double EmEnergyInEE = caloJets_it->emEnergyInEE();
//     double EmEnergyInHF = -(caloJets_it->emEnergyInHF());

//     double HadEnergyInEB = caloJets_it->hadEnergyInHB();
//     double HadEnergyInEE = caloJets_it->hadEnergyInHE();
//     double HadEnergyInHF = caloJets_it->hadEnergyInHF();

//     double EmEnergy = EmEnergyInEB + EmEnergyInEE + EmEnergyInHF;
//     double HadEnergy = HadEnergyInEB + HadEnergyInEE + HadEnergyInHF;
//     double TotalEnergy = EmEnergy + HadEnergy;

//     double EmFracFromComp = 0.;
//     if ( TotalEnergy != 0. ) EmFracFromComp = EmEnergy/(TotalEnergy);

    // Passing corrected Et, eta, phi, uncorrected Et, emEnergyFraction
    vec_IC5Jets_ptr->push_back( OfflineJet( corEt, caloJets_it->eta(), caloJets_it->phi(), caloJets_it->et(),
                                            caloJets_it->emEnergyFraction(),
//                                            EmFracFromComp,
                                            bTagHighEff_it->second, bTagHighPur_it->second,
                                            jetMass, tkNum, tkSumPt, bTagTkInvMass ) );
    ++caloJets_it;
    ++bTagHighEff_it;
    ++bTagHighPur_it;
  }

  if ( tchenum != tchpnum ) cout << "diff 1" << endl;
  if ( tchenum != iptkinfonum ) cout << "diff 2" << endl;
  if ( tchpnum != iptkinfonum ) cout << "diff 3" << endl;

  if ( tchenum != caloJetsSize ) cout << "diff jets 1" << endl;
  if ( tchpnum != caloJetsSize ) cout << "diff jets 1" << endl;
  if ( iptkinfonum != caloJetsSize ) cout << "diff jets 1" << endl;

  // Write ic5 jets
  iEvent.put( vec_IC5Jets_ptr, offlineJets_ );


  // HepMC
  // -----

  auto_ptr<MCParticleCollection> vec_MC_ptr( new MCParticleCollection );

  int MCnum = 0;
  int WtoNuNum = 0;
  int GoodWtoNuNum = 0;
  int bFromHiggsNum = 0;
  for( CandidateCollection::const_iterator MCp = MCpartons->begin(); MCp != MCpartons->end(); ++MCp ) {
    const Candidate* temp_mother = MCp->mother();
    if ( temp_mother != 0 ) {

      int pid = abs( MCp->pdgId() );
      int Mpid = abs( MCp->mother()->pdgId() );

      // If is a top(6), Z(23), W(24) or Higgs(25) or a daughter of those
      if ( pid  == 6 || pid  == 23 || pid  == 24 || pid  == 25 ||
           Mpid == 6 || Mpid == 23 || Mpid == 24 || Mpid == 25 ) {

        vec_MC_ptr->push_back( MCParticle( MCp->pt(), MCp->eta(), MCp->phi(), MCp->mass(), MCp->pdgId(), MCp->mother()->pdgId() ) );
      }

      // Count the number of nu
      if ( ( Mpid == 24 ) && ( pid ==12 || pid == 14 || pid == 16 ) ) {
        ++WtoNuNum;
        // Define analyzable neutrino decays
        // ---------------------------------
        if ( MCp->et() > 20 ) ++GoodWtoNuNum;
      }

      // Count number of b coming from Higgs
      if ( pid == 5 && Mpid == 25 ) {
        ++bFromHiggsNum;
      }
    }
    ++MCnum;
  } // end loop on MC particles

  iEvent.put( vec_MC_ptr, MCParticles_ );

  bool hTobb = false;
  // Higgs -> bbbar
  if ( bFromHiggsNum == 2 ) {
    hTobb = true;
    cout << "Higgs -> bbbar" << endl;
  }

  bool semileptonic = false;
  // Semileptonic event
  if ( WtoNuNum == 1 ) {
    semileptonic = true;
    cout << "Semileptonic event" << endl;
  }
  bool goodSemileptonic = false;
  // Semileptonic good event
  if ( GoodWtoNuNum == 1 ) {
    goodSemileptonic = true;
    cout << "Semileptonic good event" << endl;
  }


  // Pixel jets
  // ----------

  edm::Handle<PixelJetCollection> pixeljetshandle;
  edm::InputTag PixelJetsLabel = conf_.getUntrackedParameter<edm::InputTag>("PixelJetSource");
  iEvent.getByLabel(PixelJetsLabel, pixeljetshandle);

  const PixelJetCollection pixeljets = *(pixeljetshandle.product());

  auto_ptr<SimplePixelJetCollection> vec_spj_ptr( new SimplePixelJetCollection );

  PixelJetCollection::const_iterator pj_it = pixeljets.begin();
  for ( ; pj_it != pixeljets.end(); ++pj_it ) {
    vec_spj_ptr->push_back( SimplePixelJet( pj_it->pt(), pj_it->eta(), pj_it->phi(), pj_it->z(), pj_it->tkNum() ) ); 
  }

  iEvent.put( vec_spj_ptr, simplePixelJets_ );


  // ParamGlobalMuons
  // ----------------

  // See here: https://twiki.cern.ch/twiki/bin/view/CMS/MuonIsolation#Suggested_cuts
  // for possible isolation cut values.
  // From which:
  // sumPt < 3 in cone 0.3 will give a 96.7 \pm 0.7 % efficiency with about a factor of 10
  // rejection of muon candidates reconstructed in QCD events.
  // Use chi2/ndof < 10, nHits>=8 as a quality cut on reconstructed muon (applied to a
  // global-muon, this cut allows to suppress fake muons from K/pi). 

  auto_ptr<GlobalMuonCollection> vec_glbmuon_ptr( new GlobalMuonCollection );

  MuonCollection::const_iterator muon_it = paramGlobalMuons->begin();
  for ( ; muon_it != paramGlobalMuons->end(); ++muon_it ) {
    vec_glbmuon_ptr->push_back( GlobalMuon( muon_it->pt(), muon_it->eta(), muon_it->phi(), muon_it->getCalEnergy().em,
                                            muon_it->getCalEnergy().had, muon_it->getCalEnergy().ho,
                                            muon_it->getIsolationR03().sumPt, muon_it->track().get()->normalizedChi2(),
                                            muon_it->track().get()->numberOfValidHits() ) );
  }

  iEvent.put( vec_glbmuon_ptr, globalMuons_ );

  // ElectronCandidates
  // ------------------

  auto_ptr<SimpleElectronCollection> vec_simpelec_ptr( new SimpleElectronCollection );

  CandViewDoubleAssociations::const_iterator elec_it = electronHcalIsolation->begin();
  for ( ; elec_it != electronHcalIsolation->end(); ++elec_it ) {
    PixelMatchGsfElectronRef elec_ref = elec_it->first.castTo<PixelMatchGsfElectronRef>();
    const PixelMatchGsfElectron & elec = *elec_ref;
    vec_simpelec_ptr->push_back( SimpleElectron( elec.pt(), elec.eta(), elec.phi(), elec.et(), elec.hadronicOverEm(), elec_it->second ) );
  }

  iEvent.put( vec_simpelec_ptr, simpleElectrons_ );

  // Tau candidates
  // --------------

  auto_ptr<SimpleTauCollection> vec_simptau_ptr( new SimpleTauCollection );

  // -------------------------------------------------------------- //
  // Taken from Validation/RecoTau/src/TauTagVal.cc                 //
  // -------------------------------------------------------------- //
  // Other important source to look at:                             //
  // ElectroWeakAnalysis/ZTauTau_DoubleTauJet/src/EWKTauAnalyser.cc //
  // -------------------------------------------------------------- //

  const IsolatedTauTagInfoCollection & tauTagInfo = *(tauTagInfoHandle.product());
  IsolatedTauTagInfoCollection::const_iterator tauTagInfo_it = tauTagInfo.begin();
  for ( ; tauTagInfo_it != tauTagInfo.end(); ++tauTagInfo_it ) {

//     double tauJetE = tauTagInfo_it->jet()->energy();
//     double tauJetP = tauTagInfo_it->jet()->p();
//     double tauJetMass = sqrt( pow(tauJetE,2) - pow(tauJetP,2) );

    //get the tracks from the jetTag
    double tauTagTkInvMass = tagTracksInvariantMass( tauTagInfo_it->allTracks() );
    //get the selected tracks used to compute the isolation
    double tauIsoTkInvMass = tagTracksInvariantMass( tauTagInfo_it->selectedTracks() );

    vec_simptau_ptr->push_back( SimpleTau( tauTagInfo_it->jet()->pt(), tauTagInfo_it->jet()->eta(), tauTagInfo_it->jet()->phi(),
                                           tauTagTkInvMass, tauTagInfo_it->allTracks().size(),
                                           tauIsoTkInvMass, tauTagInfo_it->selectedTracks().size() ) ); 
  }

  iEvent.put( vec_simptau_ptr, simpleTaus_ );


  // Level 1 trigger
  // ---------------

  std::auto_ptr<BaseJetCollection> v_CenJets_ptr( new BaseJetCollection );
  std::auto_ptr<BaseJetCollection> v_ForJets_ptr( new BaseJetCollection );
  std::auto_ptr<BaseJetCollection> v_TauJets_ptr( new BaseJetCollection );

  // All the jets together fot the L1Trigger
  vector<SimpleJet> vec_TriggerCenJet;
  vector<SimpleJet> vec_TriggerForJet;
  vector<SimpleJet> vec_TriggerTauJet;
  for ( L1JetParticleCollection::const_iterator tcj = l1eCenJets->begin(); tcj != l1eCenJets->end(); ++tcj ) {
    vec_TriggerCenJet.push_back( SimpleJet( tcj->et(), tcj->eta(), tcj->phi() ) );
    v_CenJets_ptr->push_back( BaseJet( tcj->et(), tcj->eta(), tcj->phi() ) );
  }
  for ( L1JetParticleCollection::const_iterator tfj = l1eForJets->begin(); tfj != l1eForJets->end(); ++tfj ) {
    vec_TriggerForJet.push_back( SimpleJet( tfj->et(), tfj->eta(), tfj->phi() ) );
    v_ForJets_ptr->push_back( BaseJet( tfj->et(), tfj->eta(), tfj->phi() ) );
  }
  // Tau jets
  for ( L1JetParticleCollection::const_iterator ttj = l1eTauJets->begin(); ttj != l1eTauJets->end(); ++ttj ) {
    vec_TriggerTauJet.push_back( SimpleJet( ttj->et(), ttj->eta(), ttj->phi() ) );
    v_TauJets_ptr->push_back( BaseJet( ttj->et(), ttj->eta(), ttj->phi() ) );
  }


  // etMiss() method returns et() method (at least for version 1_7_0_pre10)
  std::auto_ptr<BaseMEt> L1MEt( new BaseMEt( l1eEtMiss->etMiss(), l1eEtMiss->phi(), 0. ) );

  // Put the L1 jest collections in the event
  iEvent.put( v_CenJets_ptr, cenJets_ );
  iEvent.put( v_ForJets_ptr, forJets_ );
  iEvent.put( v_TauJets_ptr, tauJets_ );
  try {
    iEvent.put( L1MEt, l1MEt_ );
  }
  catch (...) {
    cout << "put crash" << endl;
  }

  // Summary
  // -------

  // Set hTobb = true for qcd and "other" events
  if ( eventType_ > 7 ) hTobb = true;
  // Set semileptonic = true for all non Higgs events
  if ( eventType_ != 5 && eventType_ != 6 && eventType_ != 7 ) semileptonic = true;

  // For now fill with the number of all caloJets
  auto_ptr<Summary> summary_ptr( new Summary( eventType_, caloJetsSize, hTobb, semileptonic ) );

  iEvent.put( summary_ptr, summary_ );


#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
      ESHandle<SetupData> pSetup;
      iSetup.get<SetupRecord>().get(pSetup);
#endif
}

// ------------ method called once each job just before starting event loop  ------------
void OfflineProducer::beginJob(const edm::EventSetup&) {
}

// ------------ method called once each job just after ending the event loop  ------------
void OfflineProducer::endJob() {
}

/** Taken from CMSSW/RecoTauTag/RecoTau/src/CaloRecoTauAlgorithm.cc
 * Evaluate jet tracks invariant mass
 */
double OfflineProducer::tagTracksInvariantMass( const TrackRefVector & selectedTracks ) const {
  //      int TksQsum = 0;
  // setting invariant mass of the signal Tracks system
  math::XYZTLorentzVector mySignalTksInvariantMass(0.,0.,0.,0.);
  if( (int)(selectedTracks.size()) != 0 ) {
    int mySignalTks_qsum = 0;
    for ( int i=0; i<(int)selectedTracks.size(); ++i ) {
      mySignalTks_qsum += (selectedTracks)[i]->charge();
      math::XYZTLorentzVector mychargedpicand_fromTk_LorentzVect( (selectedTracks)[i]->momentum().x(),
                                                                  (selectedTracks)[i]->momentum().y(),
                                                                  (selectedTracks)[i]->momentum().z(),
                                                                  sqrt(pow((double)(selectedTracks)[i]->momentum().r(),2) + pow(chargedpi_mass_,2)));
      mySignalTksInvariantMass += mychargedpicand_fromTk_LorentzVect;
    }
    //        TksQsum = mySignalTks_qsum;    
  }
  return mySignalTksInvariantMass.mass();
}

//define this as a plug-in
DEFINE_FWK_MODULE(OfflineProducer);