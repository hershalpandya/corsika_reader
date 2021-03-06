import sys

try:
    import corsika
except:
    from icecube import corsika
import ROOT

# this is a ROOT option, you can try uncommenting it to see what happens
ROOT.gStyle.SetOptStat(0)

max_n = None
if len(sys.argv)>1:
    filename = sys.argv[1]
else:
    filename = corsika.example_data_dir + '/DAT000002-32'

print("opening " + filename)
f = corsika.CorsikaShowerFile(filename)

print("File has %d showers"%f.n_events)

side = 2000.
nbins = 100

distributions = {}
distributions[corsika.CorsikaParticle.Type.eElectron] = ROOT.TH2F("electrons","Electrons", nbins, -side/2, side/2, nbins, -side/2, side/2)
distributions[corsika.CorsikaParticle.Type.eMuon] = ROOT.TH2F("muons","Muons", nbins, -side/2, side/2, nbins, -side/2, side/2)
distributions[corsika.CorsikaParticle.Type.ePhoton] = ROOT.TH2F("photons","Photons", nbins, -side/2, side/2, nbins, -side/2, side/2)
distributions[corsika.CorsikaParticle.Type.eUndefined] = ROOT.TH2F("other","Other", nbins, -side/2, side/2, nbins, -side/2, side/2)

# corsika distance unit is cm
i = 0
for shower in f.events():
    for p in shower.particles:
        p_type = p.pdg_code
        if p_type in distributions.keys():
            distributions[p_type].Fill(p.x/nbins, p.y/nbins)
        else:
            distributions[corsika.CorsikaParticle.Type.eUndefined].Fill(p.x/nbins, p.y/nbins)
        i += 1
        if not max_n is None and i > max_n:
            break

c = ROOT.TCanvas()
c.Divide(2,2)
c.cd(1)
distributions[corsika.CorsikaParticle.Type.eElectron].Draw('colz')
ROOT.gPad.SetLogz()
c.cd(2)
distributions[corsika.CorsikaParticle.Type.eMuon].Draw('colz')
ROOT.gPad.SetLogz()
c.cd(3)
distributions[corsika.CorsikaParticle.Type.ePhoton].Draw('colz')
ROOT.gPad.SetLogz()
c.cd(4)
distributions[corsika.CorsikaParticle.Type.eUndefined].Draw('colz')
ROOT.gPad.SetLogz()
