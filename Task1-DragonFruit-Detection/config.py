from pprint import pprint


class Config:
    # data
    data_dir = '/home/jun/Github/DragonFruit/archive'
    min_size = 300  # image resize
    max_size = 600  # image resize
    num_workers = 1
    test_num_workers = 1

    # sigma for l1_smooth_loss
    rpn_sigma = 3.
    roi_sigma = 1.

    # param for optimizer
    # 0.0005 in origin paper but 0.0001 in tf-faster-rcnn
    weight_decay = 0.0005
    lr_decay = 0.1  # 1e-3 -> 1e-4
    lr = 1e-3

    # training
    epoch = 14

    use_adam = False  # Use Adam optimizer
    use_chainer = False  # try match everything as chainer
    use_drop = False  # use dropout in RoIHead

    def _parse(self, kwargs):
        state_dict = self._state_dict()
        for k, v in kwargs.items():
            if k not in state_dict:
                raise ValueError('UnKnown Option: "--%s"' % k)
            setattr(self, k, v)

        print('======user config========')
        pprint(self._state_dict())
        print('==========end============')

    def _state_dict(self):
        return {k: getattr(self, k) for k, _ in Config.__dict__.items() \
                if not k.startswith('_')}


opt = Config()
