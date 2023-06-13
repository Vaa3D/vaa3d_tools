import torch.nn as nn
import torch.nn.functional as F
import torch
import numpy as np


def cross_entropy_2D(input, target, weight=None, size_average=True):
    n, c, h, w = input.size()
    log_p = F.log_softmax(input, dim=1)
    log_p = log_p.transpose(1, 2).transpose(2, 3).contiguous().view(-1, c)
    target = target.view(target.numel())
    loss = F.nll_loss(log_p, target, weight=weight, size_average=False)
    if size_average:
        loss /= float(target.numel())
    return loss

def cross_entropy_3D(input, target, weight=None, size_average=True):
    n, c, h, w, s = input.size()
    log_p = F.log_softmax(input, dim=1)
    log_p = log_p.transpose(1, 2).transpose(2, 3).transpose(3, 4).contiguous().view(-1, c)
    target = target.view(target.numel())
    loss = F.nll_loss(log_p, target, weight=weight, size_average=False)
    if size_average:
        loss /= float(target.numel())
    #print(loss)
    return loss

class SoftDiceLoss(nn.Module):
    def __init__(self, weight=None, size_average=True):
        super(SoftDiceLoss, self).__init__()

    def forward(self, logits, targets):
        num = targets.size(0)
        smooth = 1

        probs = F.sigmoid(logits)
        m1 = probs.view(num, -1)
        m2 = targets.view(num, -1)
        intersection = (m1 * m2)

        score = 2. * (intersection.sum(1) + smooth) / (m1.sum(1) + m2.sum(1) + smooth)
        score = 1 - score.sum() / num
        #print(score)
        return score


class DiceMean(nn.Module):
    def __init__(self):
        super(DiceMean, self).__init__()

    def forward(self, logits, targets):
        class_num = logits.size(1)

        dice_sum = 0
        for i in range(class_num):
            inter = torch.sum(logits[:, i, :, :, :] * targets[:, i, :, :, :])
            union = torch.sum(logits[:, i, :, :, :]) + torch.sum(targets[:, i, :, :, :])
            dice = (2. * inter + 1) / (union + 1)
            dice_sum += dice
        #print(dice_sum / class_num)
        return dice_sum / class_num


class DiceMeanLoss(nn.Module):
    def __init__(self):
        super(DiceMeanLoss, self).__init__()

    def forward(self, logits, targets):
        class_num = logits.size()[1]

        dice_sum = 0
        for i in range(class_num):
            inter = torch.sum(logits[:, i, :, :] * targets[:, i, :, :])
            union = torch.sum(logits[:, i, :, :]) + torch.sum(targets[:, i, :, :])
            dice = (2. * inter + 1) / (union + 1)
            dice_sum += dice
        #print(1 - dice_sum / class_num)
        return 1 - dice_sum / class_num


class WeightDiceLoss(nn.Module):
    def __init__(self):
        super(WeightDiceLoss, self).__init__()

    def forward(self, logits, targets):

        num_sum = torch.sum(targets, dim=(0, 2, 3, 4))
        w = torch.Tensor([0.8,0.2]).cuda()
        #print(w.shape)
        #print(targets.shape)
        #print(logits.shape)
        for i in range(targets.size(1)):
            if (num_sum[i] < 1):
                w[i] = 0
            else:
                w[i] = (0.1 * num_sum[i] + num_sum[i - 1] + num_sum[i - 2] + 1) / (torch.sum(num_sum) + 1)

        inter = w * torch.sum(targets * logits, dim=(0, 2, 3, 4))
        inter = torch.sum(inter)

        union = w * torch.sum(targets + logits, dim=(0, 2, 3, 4))
        union = torch.sum(union)
        #print(1 - 2. * inter / union)

        return 1 - 2. * inter / union

def dice(logits, targets, class_index):
    inter = torch.sum(logits[:, class_index, :, :] * targets[:, class_index, :, :])
    union = torch.sum(logits[:, class_index, :, :]) + torch.sum(targets[:, class_index, :, :])
    dice = (2. * inter + 1) / (union + 1)
    #print(dice)
    return dice

def T(logits, targets):
    return torch.sum(targets[:, 2, :, :, :])

def P(logits, targets):
    return torch.sum(logits[:, 2, :, :, :])

def TP(logits, targets):
    return torch.sum(targets[:, 2, :, :, :] * logits[:, 2, :, :, :])


class BinaryDiceLoss(nn.Module):
    def __init__(self, smooth=1.0, p=1, reduction='mean', input_logits=True):
        super(BinaryDiceLoss, self).__init__()
        self.smooth = smooth
        self.p = p
        self.reduction = reduction
        self.input_logits = input_logits

    def forward(self, logits, gt_float):
        assert logits.shape[0] == gt_float.shape[0], "batch size error!"
        if self.input_logits:
            print("input_logits!")
            probs = F.softmax(logits, dim=1)[:,1]    # foreground
        else:
            probs = logits[:,1]

        probs = probs.contiguous().view(probs.shape[0], -1)
        gt_float = gt_float.contiguous().view(gt_float.shape[0], -1)
        nominator = 2 * torch.sum(torch.mul(probs, gt_float), dim=1) + self.smooth
        if self.p == 1:
            denominator = torch.sum(probs + gt_float, dim=1) + self.smooth
        elif self.p == 2:
            denominator = torch.sum(probs*probs + gt_float*gt_float, dim=1) + self.smooth
        else:
            raise NotImplementedError

        loss = 1 - nominator / denominator
        return loss.mean()

def get_fn_weights(lab_d, probs, bg_thresh=0.5, weight_fn=5.0, start_epoch=5):
    pos_mask = lab_d > 0
    bg_mask = probs[:,0] > bg_thresh            #1---for foreground; 0---for background
    fn_mask = pos_mask & bg_mask
    loss_weights = torch.ones(fn_mask.size(), dtype=probs.dtype, device=probs.device)
    loss_weights[fn_mask] = weight_fn
    loss_weights_unsq = loss_weights.unsqueeze(1)

    return loss_weights,loss_weights_unsq

def get_forward(img_d, lab_d, crit_ce, crit_dice, model):
    logits = img_d
    if isinstance(logits, list):
        weights = [1. / 2 ** i for i in range(len(logits))]
        sum_weights = sum(weights)
        weights = [w / sum_weights for w in weights]
    else:
        weights = [1.]
        logits = [logits]

    loss_ce_items, loss_dice_items = [], []

    for i in range(len(logits)):
        # get prediction
        probs = logits[i]

        # hard positive mining. NOTE: we can only do positive mining, as the label is incomplete
        do_hard_pos_mining = False
        if do_hard_pos_mining:
            loss_weights, loss_weights_unsq = get_fn_weights(lab_d, probs, bg_thresh=0.5, weight_fn=1.5, start_epoch=5)
        else:
            loss_weights = 1.0
            loss_weights_unsq = 1.0

        loss_ce = (crit_ce(logits[i], lab_d.long()) * loss_weights).mean()
        loss_dice = crit_dice(probs * loss_weights_unsq, lab_d.float() * loss_weights)
        loss_ce_items.append(loss_ce.item())
        loss_dice_items.append(loss_dice.item())
        # if i == 0:
        #     loss = (loss_ce + loss_dice) * weights[i]
        # else:
        #     loss += (loss_ce + loss_dice) * weights[i]
        loss=loss_dice+loss_ce
    return loss_ce_items, loss_dice_items, loss, logits[0]

def get_forward_eval(img_d, lab_d, crit_ce, crit_dice, model):
    with torch.no_grad():
        loss_ces, loss_dices, loss, logits = get_forward(img_d, lab_d, crit_ce, crit_dice, model)
    return loss_ces, loss_dices, loss, logits
