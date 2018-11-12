load('SysId.mat')
n = size(SysId,1) - 1;
V = SysId(2:end,2);
phi = [SysId(1:n,2) 6*ones(n,1)];
c = phi\V;

c1 = c(1);
c2 = c(2);

disp(c);

car_tf = tf(c2,[1 -c1],0.005);

figure
opt = stepDataOptions('StepAmplitude',6);
step(car_tf,4,opt);
hold on
plot(SysId(:,1),SysId(:,2),'r');
legend('Output of Model', 'Actual Response');
hold off