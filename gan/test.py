#!/usr/bin/python3

import argparse
import sys
import os

import torchvision.transforms as transforms
from torchvision.utils import save_image
from torch.utils.data import DataLoader
import torch

from model.models import Generator, StyleGanGenerator
from model.models import Discriminator
from datasets import ImageDataset, SequentialImageDataset

parser = argparse.ArgumentParser()
parser.add_argument('--batchSize', type=int, default=1, help='size of the batches')
parser.add_argument('--dataroot', type=str, default='datasets/horse2zebra/', help='root directory of the dataset')
parser.add_argument('--input_nc', type=int, default=3, help='number of channels of input data')
parser.add_argument('--output_nc', type=int, default=3, help='number of channels of output data')
parser.add_argument('--size', type=int, default=128, help='size of the data (squared assumed)')
parser.add_argument('--cuda', action='store_true', help='use GPU computation')
parser.add_argument('--n_cpu', type=int, default=8, help='number of cpu threads to use during batch generation')
parser.add_argument('--generator_A2B', type=str, default='output/199_netG_A2B.pth', help='A2B generator checkpoint file')
parser.add_argument('--generator_B2A', type=str, default='output/199_netG_B2A.pth', help='B2A generator checkpoint file')
parser.add_argument('--generator_S2P', type=str, default='output/199_netG_S2P.pth', help='S2P generator checkpoint file')
parser.add_argument('--sequential', action='store_true', help='if the dataset have a sequence')
parser.add_argument('--ske', action='store_true', help='if the dataset have a skeleton input')
parser.add_argument('--output_dir', type=str, default='./output', help='place to output result')
opt = parser.parse_args()

opt.generator_A2B = opt.generator_A2B.replace('output', opt.output_dir)
opt.generator_B2A = opt.generator_B2A.replace('output', opt.output_dir)
opt.generator_S2P = opt.generator_S2P.replace('output', opt.output_dir)

print(opt)

if torch.cuda.is_available() and not opt.cuda:
    print("WARNING: You have a CUDA device, so you should probably run with --cuda")

###### Definition of variables ######
# Networks
netG_A2B = Generator(opt.input_nc, opt.output_nc)
netG_B2A = Generator(opt.output_nc, opt.input_nc)
netG_S2P = GanGenerator(opt.output_nc, opt.input_nc, True)
netG_P2S = GanGenerator(opt.output_nc, opt.input_nc, True)

if opt.cuda:
    netG_A2B.cuda() #netG_A2B.to(torch.device('cuda')) new
    netG_B2A.cuda() #netG_B2A.to(torch.device('cuda'))
    netG_S2P.cuda()

# Load state dicts
if opt.cuda:
    netG_A2B.load_state_dict({k.replace('module.',''):v for k,v in torch.load(opt.generator_A2B).items()})
    netG_B2A.load_state_dict({k.replace('module.',''):v for k,v in torch.load(opt.generator_B2A).items()})
    if opt.ske:
        netG_S2P.load_state_dict({k.replace('module.',''):v for k,v in torch.load(opt.generator_S2P).items()})
else:
    netG_A2B.load_state_dict({k.replace('module.', ''): v for k, v in torch.load(opt.generator_A2B, map_location=torch.device('cpu')).items()})
    netG_B2A.load_state_dict({k.replace('module.', ''): v for k, v in torch.load(opt.generator_B2A, map_location=torch.device('cpu')).items()})
    if opt.ske:
        netG_S2P.load_state_dict({k.replace('module.', ''): v for k, v in torch.load(opt.generator_S2P, map_location=torch.device('cpu')).items()})

# Set model's test mode e.x.  set dropout and batch normalization layers to evaluation mode
netG_A2B.eval()
netG_B2A.eval()

# Inputs & targets memory allocation
Tensor = torch.cuda.FloatTensor if opt.cuda else torch.Tensor
input_A = Tensor(opt.batchSize, opt.input_nc, opt.size, opt.size)
input_B = Tensor(opt.batchSize, opt.output_nc, opt.size, opt.size)

# Dataset loader
transforms_ = [ transforms.ToTensor(),
                transforms.Normalize((0.5,), (0.5,)) ]
if opt.sequential:
    dataloader = DataLoader(SequentialImageDataset(opt.dataroot, transforms_=transforms_, mode='test'),
                        batch_size=opt.batchSize, shuffle=False, num_workers=opt.n_cpu)
else:
    dataloader = DataLoader(ImageDataset(opt.dataroot, transforms_=transforms_, mode='test'),
                        batch_size=opt.batchSize, shuffle=False, num_workers=opt.n_cpu)
###################################

###### Testing######

# Create output dirs if they don't exist
if not os.path.exists(opt.output_dir + '/A'):
    os.makedirs(opt.output_dir +'/A')
if not os.path.exists(opt.output_dir + '/B'):
    os.makedirs(opt.output_dir +'/B')
if not os.path.exists(opt.output_dir + '/P'):
    os.makedirs(opt.output_dir +'/P')

if opt.sequential and opt.ske:
    for i, batch in enumerate(dataloader):
        # Set model input
        real_B = batch['B'].cuda()
        A_traj = batch['A_traj'].cuda()
        A_comp_0 = batch['A_comp_0'].cuda()
        A_comp_1 = batch['A_comp_1'].cuda()
        A_comp_1_1 = batch['A_comp_1_1'].cuda()
        A_comp_2 = batch['A_comp_2'].cuda()
        A_comp_3 = batch['A_comp_3'].cuda()

        # Generate output
        fake_B = 0.5*(netG_A2B(A_comp_1).data + 1.0)
        fake_P = 0.5*(netG_S2P(A_full, A_ske).data + 1.0)
        # fake_A = 0.5*(netG_B2A(real_B).data + 1.0)

        # Save image files
        save_image(fake_P, opt.output_dir + '/P/%04d.png' % (i+1))
        save_image(fake_B, opt.output_dir + '/B/%04d.png' % (i+1))

        sys.stdout.write('\rGenerated images %04d of %04d' % (i+1, len(dataloader)))

    sys.stdout.write('\n')

elif opt.sequential:

    for i, batch in enumerate(dataloader):
        # Set model input
        real_A = batch['A_label'].cuda()
        A_part = batch['A_part'].cuda()
        A_full = batch['A_full'].cuda()
        real_B = batch['B'].cuda()

        # Generate output
        fake_B = 0.5*(netG_A2B.forward_with_stroke(A_full, A_part).data + 1.0)
        # fake_A = 0.5*(netG_B2A(real_B).data + 1.0)

        # Save image files
        # save_image(fake_A, 'output/A/%04d.png' % (i+1))
        save_image(fake_B, 'output/B/%04d.png' % (i+1))

        sys.stdout.write('\rGenerated images %04d of %04d' % (i+1, len(dataloader)))

    sys.stdout.write('\n')
else:
    for i, batch in enumerate(dataloader):
        # Set model input
        real_A = input_A.copy_(batch['A'])
        real_B = input_B.copy_(batch['B'])

        # Generate output
        fake_B = 0.5*(netG_A2B(real_A).data + 1.0)
        fake_A = 0.5*(netG_B2A(real_B).data + 1.0)

        # Save image files
        save_image(fake_A, 'output/A/%04d.png' % (i+1))
        save_image(fake_B, 'output/B/%04d.png' % (i+1))

        sys.stdout.write('\rGenerated images %04d of %04d' % (i+1, len(dataloader)))

    sys.stdout.write('\n')
###################################
