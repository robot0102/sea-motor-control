import torch
import logging

import torchvision.transforms as transforms
import numpy as np

from utils import skeletonize, stroke2img


def load_class(class_name):
    components = class_name.split('.')
    mod = __import__(components[0])
    for comp in components[1:]:
        mod = getattr(mod, comp)
    return mod

class controller(object):

    def __init__(self) -> None:
        pass


class Learner(object):
    """ class that stores learner performance 
    """
    def __init__(self) -> None:
        
        self.__init_parameters()
    
    def __init_parameters(self,):
        self.score = 0
        self.satisfied = False
    
    def reset(self):

        self.__init_parameters()

class Executor(object):
    """Class that carries out teaching process

    Args:
        object ([type]): [description]
    """
    def __init__(self, args) -> None:   

        logging.info('Initialize Runner')
        self.cuda = args.get('CUDA', False)
        self.feedbaock = args.get('WITH_FEEDBACK', False)
        self.learner = Learner()
        self.controller = controller()

        self.__init_parameters(args)
        self.__init_network()

    def __init_parameters(self,args):

        self.gan_path = args.get('GAN_MODEL_PATH')
        self.dis_path = args.get('DIS_MODEL_PATH')

        self.gan_type = args.get('GAN_MODEL_TYPE')
        self.dis_type = args.get('DIS_MODEL_TYPE')

        self.input_channel = args.get('INPUT_CHANNEL')
        self.output_channel = args.get('OUTPUT_CHANNEL')

        self.font_type = args.get('TTF_FILE')
        self.font_size = args.get('FONT_SIZE', 128)
        assert self.font_type is not None, 'Please provide a font file'

        if args.get('PRE_PROCESS',None):
            assert args.get('PRE_PROCESS').upper() == 'DEFAULT', '{} preprocess is not supported'.format(args.get('PRE_PROCESS'))
            self.pre_process = [ transforms.ToTensor(),
                                 transforms.Normalize((0.5,), (0.5,)) ]
        else:
            self.pre_process = [ transforms.ToTensor() 
                                ] 

    def __init_network(self,):

        self.gan = load_class(self.gan_type)(self.input_channel, self.output_channel)
        self.dis = load_class(self.dis_type)(self.output_channel)

        if self.cuda: 
            self.gan = self.gan.cuda()
            self.dis = self.dis.cuda()
        
        if self.gan_path is not None:
            self.gan.load_state_dict({k.replace('module.',''):v for k,v in torch.load(self.gan_path).items()})

        if self.dis_path is not None:
            self.dis.load_state_dict({k.replace('module.',''):v for k,v in torch.load(self.dis_path).items()})

    def interact(self, traj, score=None):
        """TO DO: interaction part
        """
        output_img = self.__capture_image()
        self.learner.score = self.get_score(output_img)
        return False

    def get_score(self, image): 

        return self.dis(self.pre_process(image))
    
    def __capture_image(self, ):
        """ Capture image with post process in order for discrinmintor to score
        """
        raise NotImplementedError

    def sample_stroke(self, ):
        """ For future development, decompose one character into several strokes
        """
        pass

    def __reset_learner(self,):
        """ Reset learner model
        """
        self.learner.reset()
    
    def __quit(self):
        """ Quit all the processes
        """
        pass

    def pipeline(self,):
        """ Full pipeline
        Obtain target stroke -> generate target stroke's trajectory -> Interact with learner -> Get learner output
        """

        while True:

            stroke = input('Please provide a stroke you want to learn: ')

            if stroke is ' ':
                break

            while not self.learner.satisfied: 

                stroke_img = stroke2img(self.font_type, stroke,self.font_size)
                stroke_img = np.array(stroke_img)
                traj, traj_img = skeletonize(~stroke_img)

                self.interact(traj)

        logging.info('Quittiing')

        # cv2.imshow('',stroke_img)
        # cv2.waitKey(0)