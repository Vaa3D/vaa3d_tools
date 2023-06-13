import torch
from models.soma_unet import unet3d
import config

def main():
    # args = config.args
    # device = torch.device('cpu' if args.cpu else 'cuda')
    # model = unet3d(1, [8, 16, 32, 64, 128], 1).to(device)
    # ckpt = torch.load('./output/best_model.pth')
    # model.load_state_dict(ckpt['net'])
    model = unet3d(1, [8, 16, 32, 64, 128], 1)
    model.eval()
    example = torch.rand(1, 1, 128, 128, 128)
    # Use torch.jit.trace to generate a torch.jit.ScriptModule via tracing.
    traced_script_module = torch.jit.trace(model, example)
    # Serializing Your Script Module to a File
    traced_script_module.save("traced_unet_model.pt")
    print('finish!')

if __name__=="__main__":
    main()