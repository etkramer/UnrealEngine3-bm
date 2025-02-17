class AIReactCond_Kantus_EnemyCloseEnoughToScream extends AIReactCond_EnemyCloseAndVisible
	within GearAI_Kantus;


event bool ShouldActivate( Actor EventInstigator, AIReactChannel OrigChan )
{
	return (Super.ShouldActivate(EventInstigator,OrigChan) && CanDoKnockdown());
}
